#include <string.h>
#include <stdio.h>

#include <inttypes.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include <esp_netif.h>
#include <esp_err.h>
#include "esp_chip_info.h"
#include "esp_flash.h"


#include "nvs_flash.h"
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip4_addr.h"

#include "driver/gpio.h"
#include "driver/uart.h"


#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 21
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27
#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static const char *TAG = "wifi station";
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Led's configuration
#define LED_RED_PIN    GPIO_NUM_32
#define LED_YELLOW_PIN GPIO_NUM_33

// Lora pins configuration
#define TXD_PIN GPIO_NUM_2
#define RXD_PIN GPIO_NUM_15
#define UART_NUM UART_NUM_2
#define BUF_SIZE 0x200


static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Set the response content type as JPEG
    httpd_resp_set_type(req, "image/jpeg");

    // Send the image
    esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    
    esp_camera_fb_return(fb);

    return res;
}

// HTTP GET handler for root route
esp_err_t index_handler(httpd_req_t *req)
{
    const char* response = "<html><body><h1>ESP32 Camera</h1><img src=\"/capture\" width=\"640\" height=\"480\"></body></html>";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;  // Add this line to indicate success
}

// Define URI handlers
httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
};

httpd_uri_t capture_uri = {
    .uri       = "/capture",
    .method    = HTTP_GET,
    .handler   = capture_handler,
    .user_ctx  = NULL
};

static esp_err_t start_http_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the HTTP server
    httpd_handle_t server = NULL;
    esp_err_t res = httpd_start(&server, &config);

    if (res == ESP_OK) {
        // Register URI handlers
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &capture_uri);
    }

    return res;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", EXAMPLE_ESP_WIFI_SSID);
    } else {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", EXAMPLE_ESP_WIFI_SSID);
    }
}

static esp_err_t init_camera(void)
{
    camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,
        .jpeg_quality = 12,
        .fb_count = 1
    };
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }
    return ESP_OK;
}

size_t command(char* response)
{
	
	size_t len = uart_read_bytes(UART_NUM, response, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
	
	if (len)
	{
		response[len] = 0;
	}
	
	return len;
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    
    ESP_ERROR_CHECK(init_camera());
    
    ESP_LOGI(TAG, "Starting HTTP server...");
    ESP_ERROR_CHECK(start_http_server());

    gpio_set_direction(LED_RED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_YELLOW_PIN, GPIO_MODE_OUTPUT);

const uart_config_t uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_DEFAULT,
	};

	uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

	uart_param_config(UART_NUM, &uart_config);

	uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	char response[BUF_SIZE];
	
	
	
	int i = 0;

	for (;;)
	{
			
		size_t len = command(response);

		if (len)
		{
			printf("Received: %s\n", response);

			if (strstr(response, "RED_LED"))
			{
				gpio_set_level(LED_RED_PIN, 1);
			        gpio_set_level(LED_YELLOW_PIN, 0);
				vTaskDelay(2000 / portTICK_PERIOD_MS);
				gpio_set_level(LED_RED_PIN, 0);

			}
			else if (strstr(response, "YEL_LED"))
			{
				gpio_set_level(LED_RED_PIN, 0);
			        gpio_set_level(LED_YELLOW_PIN, 1);
				vTaskDelay(2000 / portTICK_PERIOD_MS);
				gpio_set_level(LED_YELLOW_PIN, 0);

			}
			else if (strstr(response, "BTH_LED"))
			{
				gpio_set_level(LED_RED_PIN, 1);
			        gpio_set_level(LED_YELLOW_PIN, 1);
				vTaskDelay(2000 / portTICK_PERIOD_MS);
				gpio_set_level(LED_RED_PIN, 0);
				gpio_set_level(LED_YELLOW_PIN, 0);

			}

			vTaskDelay(2000 / portTICK_PERIOD_MS);


		}
		
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}


}
