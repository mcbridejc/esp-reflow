#include "Control.h"
#include "ProfileManager.h"

#include "esp_http_server.h"

class HttpServer {
public:
    HttpServer(Control *control, ProfileManager *profile_manager);

    void init();

    static esp_err_t GetStatus(httpd_req_t *req);
    static esp_err_t GetStart(httpd_req_t *req);
    static esp_err_t GetStop(httpd_req_t *req);
    static esp_err_t GetTempHold(httpd_req_t *req);
    static esp_err_t GetLog(httpd_req_t *req);
    static esp_err_t ProfilesIndex(httpd_req_t *req);
    static esp_err_t ProfilesCreate(httpd_req_t *req);
    static esp_err_t ProfilesUpdate(httpd_req_t *req);
    static esp_err_t ProfilesDestroy(httpd_req_t *req);
    static esp_err_t ProfilesActive(httpd_req_t *req);
    static esp_err_t GetActivate(httpd_req_t *req);

    static esp_err_t GetFile(httpd_req_t *req);
private:
    Control *mControl;
    ProfileManager *mProfileManager;
    httpd_handle_t mServer;

    void RegisterGet(const char *uri, esp_err_t (*handler)(httpd_req_t *r));
    static std::string GetLastPathNode(const char *path);

    std::string SerializedProfiles();
};
