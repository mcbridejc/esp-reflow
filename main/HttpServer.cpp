#include "HttpServer.h"

#include "json/json.h"

#include "esp_log.h"

#include <string>

#define JSON_BUFFER_SIZE 4096

static const char *TAG = "HttpServer";

HttpServer::HttpServer(
        Control *control,
        ProfileManager *profile_manager,
        ReflowLog *log)
{
    mControl = control;
    mProfileManager = profile_manager;
    mLog = log;
}

void HttpServer::init() {
    httpd_uri_t route;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.uri_match_fn = httpd_uri_match_wildcard;
    cfg.max_uri_handlers = 20;

    ESP_ERROR_CHECK(httpd_start(&mServer, &cfg));

    RegisterGet("/api/status", HttpServer::GetStatus);
    RegisterGet("/api/temphold", HttpServer::GetTempHold);
    RegisterGet("/api/stop", HttpServer::GetStop);
    RegisterGet("/api/start", HttpServer::GetStart);
    RegisterGet("/api/log", HttpServer::GetLog);
    
    RegisterGet("/api/activate/*", HttpServer::GetActivate);

    // Profile management routes
    RegisterGet("/api/profiles", HttpServer::ProfilesIndex);

    RegisterGet("/api/profiles/active", HttpServer::ProfilesActive);

    route = {
        .uri = "/api/profiles",
        .method = HTTP_POST,
        .handler = HttpServer::ProfilesCreate,
        .user_ctx = this
    };
    httpd_register_uri_handler(mServer, &route);

    route = {
        .uri = "/api/profiles/*",
        .method = HTTP_PUT,
        .handler = HttpServer::ProfilesUpdate,
        .user_ctx = this
    };
    httpd_register_uri_handler(mServer, &route);

    route = {
        .uri = "/api/profiles/*",
        .method = HTTP_DELETE,
        .handler = HttpServer::ProfilesDestroy,
        .user_ctx = this
    };
    httpd_register_uri_handler(mServer, &route);

    // Now register a catch-all to serve files
    route = {
        .uri = "*",
        .method = HTTP_GET,
        .handler = HttpServer::GetFile,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(mServer, &route);
}

void HttpServer::RegisterGet(const char *uri, esp_err_t (*handler)(httpd_req_t *r)) {
    httpd_uri_t route;
    route.uri = uri;
    route.method = HTTP_GET;
    route.handler = handler;
    route.user_ctx = this;

    httpd_register_uri_handler(mServer, &route);
}

esp_err_t HttpServer::GetStatus(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    Json::Value root;
    root["temperature"] = ctx->mControl->latestTemp();
    root["targetTemperature"] = ctx->mControl->targetTemp();
    root["profileStage"] = ctx->mControl->profileStage();
    root["profileElapsedTime"] = ctx->mControl->profileElapsedTime();
    root["output"] = ctx->mControl->output();
    Control::State_e state = ctx->mControl->currentState();
    switch(state) {
    case Control::Idle:
        root["state"] = "idle";
        break;
    case Control::RunningProfile:
        root["state"] = "running";
        break;
    case Control::RunningTempHold:
        root["state"] = "tempHold";
        break;
    default:
        root["state"] = "unknown";
        break;
    }

    Json::FastWriter writer;
    std::string jsondata = writer.write(root);
    httpd_resp_send(req, &jsondata[0], jsondata.size());
    return ESP_OK;
}

esp_err_t HttpServer::GetTempHold(httpd_req_t *req) {
    esp_err_t err;
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    uint32_t buf_len = httpd_req_get_url_query_len(req) + 1;
    int16_t tempHold = 0x7FFF;
    if(buf_len > 1) {
        char *buf = (char *)malloc(buf_len);
        if((err = httpd_req_get_url_query_str(req, buf, buf_len)) == ESP_OK) {
            char param[8];
            if(httpd_query_key_value(buf, "temp", param, sizeof(param)) == ESP_OK) {
                tempHold = atoi(param);
            }
        }
        free(buf);
    }
    if(tempHold == 0x7FFF) {
        httpd_resp_set_status(req, "400 Bad Request");
        const char *resp = "You did not provide the temp query param";
        httpd_resp_send(req, resp, strlen(resp));
        return ESP_OK;
    }

    ctx->mControl->holdTemp((float)tempHold);
    char respBuf[64];
    snprintf(respBuf, sizeof(respBuf), "Setting temp to %d", tempHold);
    httpd_resp_send(req, respBuf, strlen(respBuf));

    return ESP_OK;
}

esp_err_t HttpServer::GetLog(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    Json::Value root = Json::objectValue;
    Json::FastWriter jsonWriter;

    root["timestamp_ms"] = Json::arrayValue;
    root["integratorSum"] = Json::arrayValue;
    root["measuredTemperature"] = Json::arrayValue;
    root["targetTemperature"] = Json::arrayValue;
    root["output"] = Json::arrayValue;
    for(int i=0; i<ctx->mLog->size(); i++) {
        ReflowLog::Entry &e = (*ctx->mLog)[i];
        root["timestamp_ms"][i] = e.timestamp_ms;
        root["integratorSum"][i] = e.integratorSum;
        root["measuredTemperature"][i] = e.measuredTemp;
        root["targetTemperature"][i] = e.targetTemp;
        root["output"][i] = e.output;
    }

    std::string resp = jsonWriter.write(root);
    httpd_resp_send(req, &resp[0], resp.size());
    return ESP_OK;
}

esp_err_t HttpServer::GetStart(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    ctx->mControl->startProfile();
    ctx->mLog->reset();

    const char *resp = "Starting profile";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t HttpServer::GetStop(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    ctx->mControl->stop();

    const char *resp = "Stopping controller";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t HttpServer::GetActivate(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    
    std::string name = GetLastPathNode(req->uri);
    bool success = ctx->mProfileManager->setActiveProfile(name.c_str());
    if(!success) {
        ESP_LOGW(TAG, "Failed to set active profile to %s", name.c_str());
        httpd_resp_send_500(req);
        return ESP_OK;
    }
    ctx->mControl->setProfile(ctx->mProfileManager->getActiveProfile());
    const char *resp = "Set active profile";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t HttpServer::ProfilesActive(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    Json::Value root;
    Json::FastWriter jsonWriter;

    Profile &activeProfile = ctx->mProfileManager->getActiveProfile();
    root["name"] = activeProfile.name();
    root["steps"] = Json::arrayValue;
    for(int i=0; i<activeProfile.size(); i++) {
        root["steps"][i]["temp"] = activeProfile[i].temp;
        root["steps"][i]["duration"] = activeProfile[i].duration;
        root["steps"][i]["ramp"] = activeProfile[i].ramp;
    }
    std::string resp = jsonWriter.write(root);
    httpd_resp_send(req, &resp[0], resp.size());
    return ESP_OK;
}

esp_err_t HttpServer::ProfilesCreate(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    Json::Value root;
    Json::Reader jsonReader;
    char *recvBuf = (char *)malloc(req->content_len);
    if(recvBuf == NULL) {
        return ESP_FAIL;
    }
    httpd_req_recv(req, recvBuf, req->content_len);
    bool success = jsonReader.parse(recvBuf, recvBuf + req->content_len, root, false);
    free(recvBuf);
    if(!success) {
        ESP_LOGE(TAG, "Failed to parse json in profiles create request");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Creating profile %s", root["name"].asString().c_str());

    Profile newProfile(root["name"].asString().c_str());
    for(int i=0; i<root["steps"].size(); i++) {
        Json::Value &jsonStep = root["steps"][i];
        ProfileStep step = {
            .temp = (uint8_t)jsonStep["temp"].asInt(),
            .duration = (uint8_t)jsonStep["duration"].asInt(),
            .ramp = (uint8_t)jsonStep["ramp"].asInt()
        };
        newProfile.addStep(step);
    }
    success = ctx->mProfileManager->createProfile(newProfile);

    if(!success) {
        ESP_LOGE(TAG, "Error creating profile %s", newProfile.name());
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    std::string jsondata = ctx->SerializedProfiles();
    httpd_resp_send(req, &jsondata[0], jsondata.size());
    return ESP_OK;
}

std::string HttpServer::SerializedProfiles() {
    Json::Value root = Json::arrayValue;
    Json::FastWriter jsonWriter;

    std::vector<Profile> profiles = mProfileManager->getAllProfiles();
    for(int i=0; i<profiles.size(); i++) { 
        root[i]["name"] = profiles[i].name();
        root[i]["steps"] = Json::arrayValue;
        for(int j=0; j<profiles[i].size(); j++) {
            root[i]["steps"][j]["temp"] = profiles[i][j].temp;
            root[i]["steps"][j]["duration"] = profiles[i][j].duration;
            root[i]["steps"][j]["ramp"] = profiles[i][j].ramp;
        }
    }

    return jsonWriter.write(root);
}

esp_err_t HttpServer::ProfilesIndex(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    std::string jsondata = ctx->SerializedProfiles();
    httpd_resp_send(req, &jsondata[0], jsondata.size());
    return ESP_OK;
}

esp_err_t HttpServer::ProfilesUpdate(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    Json::Value root;
    Json::Reader jsonReader;

    // Read body and parse as JSON
    char *recvBuf = (char *)malloc(req->content_len);
    if(recvBuf == NULL) {
        ESP_LOGE(TAG, "Failled to allocate buffer of size %d for HTTP receive content", req->content_len);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    httpd_req_recv(req, recvBuf, req->content_len);
    bool success = jsonReader.parse(recvBuf, recvBuf + req->content_len, root, false);
    free(recvBuf);
    
    if(!success) {
        ESP_LOGE(TAG, "Failed to parse json in profiles create request");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Name of the profile to update is provided in the url: `/api/profiles/:name`
    std::string profileName = GetLastPathNode(req->uri);

    ESP_LOGI(TAG, "Updating profile %s", profileName.c_str());

    Profile newProfile(root["name"].asString().c_str());
    for(int i=0; i<root["steps"].size(); i++) {
        Json::Value &jsonStep = root["steps"][i];
        ProfileStep step = {
            .temp = (uint8_t)jsonStep["temp"].asInt(),
            .duration = (uint8_t)jsonStep["duration"].asInt(),
            .ramp = (uint8_t)jsonStep["ramp"].asInt()
        };
        newProfile.addStep(step);
    }

    ESP_LOGI(TAG, "Updating profile %s", profileName.c_str());
    success = ctx->mProfileManager->updateProfile(profileName.c_str(), newProfile);
    if(!success) {
        httpd_resp_send_500(req);
        return ESP_OK;
    }
    
    std::string jsondata = ctx->SerializedProfiles();
    httpd_resp_send(req, &jsondata[0], jsondata.size());
    return ESP_OK;
}

esp_err_t HttpServer::ProfilesDestroy(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;
    std::string name = GetLastPathNode(req->uri);
    bool success = ctx->mProfileManager->deleteProfile(name.c_str());
    if(!success) {
        httpd_resp_send_500(req);
        return ESP_OK;
    }

    std::string jsondata = ctx->SerializedProfiles();
    httpd_resp_send(req, &jsondata[0], jsondata.size());    
    return ESP_OK;
}

esp_err_t HttpServer::GetFile(httpd_req_t *req) {
    const int CHUNKSIZE = 4096;
    std::string path = "/www";
    if(strcmp("/", req->uri) == 0) {
        path += "/index.html";
    } else {
        path += req->uri;
    }

    FILE* f = fopen(path.c_str(), "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", path.c_str());
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    char *chunk = (char *)malloc(CHUNKSIZE);
    if(chunk == NULL) {
        return ESP_FAIL;
    }
    int readlength = 0xffff;
    while(readlength > 0) {
        readlength = fread(chunk, 1, CHUNKSIZE, f);
        httpd_resp_send_chunk(req, chunk, readlength);
    }
    // Call with 0 data to finish the transmission and close socket
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

std::string HttpServer::GetLastPathNode(const char *path) {
    size_t len = strlen(path);
    // Ignore trailing slash if present
    if(path[len-1] == '/') {
        len--; 
    }
    size_t idx = len-1;
    while(path[idx] != '/' && idx > 0) {
        idx--;
    }
    
    // Sometimes we stop on a slash, but sometimes we stop on the first character
    // In the former case, we don't actually want to include the / in return value
    if(path[idx] == '/') {
        idx++;
    }
    return std::string(path + idx, len-idx);
}