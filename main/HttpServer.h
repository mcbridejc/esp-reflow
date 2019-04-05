#include "Control.h"

#include "esp_http_server.h"

class HttpServer {
public:
    HttpServer(Control *control);

    void init();

    static esp_err_t GetStatus(httpd_req_t *req);
    static esp_err_t GetStart(httpd_req_t *req);
    static esp_err_t GetStop(httpd_req_t *req);
    static esp_err_t GetTempHold(httpd_req_t *req);
    static esp_err_t GetActiveProfile(httpd_req_t *req);
    static esp_err_t GetLog(httpd_req_t *req);    

    static esp_err_t GetFile(httpd_req_t *req);
private:
    Control *mControl;
    httpd_handle_t mServer;

    void RegisterGet(const char *uri, esp_err_t (*handler)(httpd_req_t *r));
};
