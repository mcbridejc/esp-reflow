#include "HttpServer.h"

#include "json/json.h"

#include <string>

#define JSON_BUFFER_SIZE 4096

HttpServer::HttpServer(Control *control) : mControl(control) 
{

}

void HttpServer::init() {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&mServer, &cfg));

    RegisterGet("/api/status", HttpServer::GetStatus);
    RegisterGet("/api/temphold", HttpServer::GetTempHold);
    RegisterGet("/api/stop", HttpServer::GetStop);
    RegisterGet("/api/start", HttpServer::GetStart);
    //RegisterGet("/activeprofile", HttpServer::GetActiveProfile);
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

esp_err_t HttpServer::GetStart(httpd_req_t *req) {
    HttpServer *ctx = (HttpServer*)req->user_ctx;

    ctx->mControl->startProfile();

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