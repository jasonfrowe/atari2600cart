/*
* PicoMultiRom for ATARI 2600+
* Jason Rowe (jasonfrowe@gmail.com)
* Now with WIFI!! 
* January 2024
*/

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pin_definitions.h"
#include <stdlib.h>
#include "hardware/adc.h"

#include "pico/cyw43_arch.h" // Library for PICO_W 

#include "lwip/dns.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"

#define DEBUG_printf printf

#define MQTT_SERVER_HOST "pi5.home.arpa" //A Raspberry Pi running Mosquitto 
#define MQTT_SERVER_PORT 1883            //Port for MQTT

// #define MEMP_NUM_SYS_TIMEOUT (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 100)

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Perform initialisation
static MQTT_CLIENT_T* mqtt_client_init(void) {
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    state->received = 0;
    return state;
}

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    DEBUG_printf("DNS query finished with resolved addr of %s.\n", ip4addr_ntoa(ipaddr));
    state->remote_addr = *ipaddr;
}

void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Running DNS query for %s.\n", MQTT_SERVER_HOST);

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_ARG) {
        DEBUG_printf("failed to start DNS query\n");
        return;
    }

    if (err == ERR_OK) {
        DEBUG_printf("no lookup needed");
        return;
    }

    while (state->remote_addr.addr == 0) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
}


u32_t data_in = 0;

u8_t buffer[1025];
u8_t data_len = 0;

static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("mqtt_pub_start_cb: topic %s\n", topic);

    if (tot_len > 1024) {
        DEBUG_printf("Message length exceeds buffer size, discarding");
    } else {
        data_in = tot_len;
        data_len = 0;
    }
}

static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    if (data_in > 0) {
        data_in -= len;
        memcpy(&buffer[data_len], data, len);
        data_len += len;

        if (data_in == 0) {
            buffer[data_len] = 0;
            DEBUG_printf("Message received: %s\n", &buffer);
        }
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status != 0) {
        DEBUG_printf("Error during connection: err %d.\n", status);
    } else {
        DEBUG_printf("MQTT connected.\n");
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;
    DEBUG_printf("mqtt_pub_request_cb: err %d\n", err);
    state->received++;
}

void mqtt_sub_request_cb(void *arg, err_t err) {
    DEBUG_printf("mqtt_sub_request_cb: err %d\n", err);
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state)
{
  char buffer[128];

  sprintf(buffer, "{\"message\":\"hello from picow %d / %d\"}", state->received, state->counter);

  err_t err;
  u8_t qos = 0; /* 0 1 or 2, see MQTT specification.  AWS IoT does not support QoS 2 */
  u8_t retain = 0;
  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, "pico_w/test", buffer, strlen(buffer), qos, retain, mqtt_pub_request_cb, state);
  cyw43_arch_lwip_end();
  if(err != ERR_OK) {
    DEBUG_printf("Publish err: %d\n", err);
  }

  return err; 
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci;
    err_t err;

    memset(&ci, 0, sizeof(ci));

    ci.client_id = "Atari2600";
    ci.client_user = NULL;
    ci.client_pass = NULL;
    ci.keep_alive = 0;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

    #if MQTT_TLS

    struct altcp_tls_config *tls_config;
  
    #if defined(CRYPTO_CA) && defined(CRYPTO_KEY) && defined(CRYPTO_CERT)
    DEBUG_printf("Setting up TLS with 2wayauth.\n");
    tls_config = altcp_tls_create_config_client_2wayauth(
        (const u8_t *)ca, 1 + strlen((const char *)ca),
        (const u8_t *)key, 1 + strlen((const char *)key),
        (const u8_t *)"", 0,
        (const u8_t *)cert, 1 + strlen((const char *)cert)
    );
    // set this here as its a niche case at the moment.
    // see mqtt-sni.patch for changes to support this.
    ci.server_name = MQTT_SERVER_HOST;
    #elif defined(CRYPTO_CERT)
    DEBUG_printf("Setting up TLS with cert.\n");
    tls_config = altcp_tls_create_config_client((const u8_t *) cert, 1 + strlen((const char *) cert));
    #endif

    if (tls_config == NULL) {
        DEBUG_printf("Failed to initialize config\n");
        return -1;
    }

    ci.tls_config = tls_config;
    #endif

    const struct mqtt_connect_client_info_t *client_info = &ci;

    err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, client_info);
    
    if (err != ERR_OK) {
        DEBUG_printf("mqtt_connect return %d\n", err);
    }

    return err;
}

void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();

    state->counter = 0;  

    if (state->mqtt_client == NULL) {
        DEBUG_printf("Failed to create new mqtt client\n");
        return;
    } 
    // psa_crypto_init();
    if (mqtt_test_connect(state) == ERR_OK) {
        absolute_time_t timeout = nil_time;
        bool subscribed = false;
        mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb, mqtt_pub_data_cb, 0);

        while (true) {
            cyw43_arch_poll();
            absolute_time_t now = get_absolute_time();
            if (is_nil_time(timeout) || absolute_time_diff_us(now, timeout) <= 0) {
                if (mqtt_client_is_connected(state->mqtt_client)) {
                    cyw43_arch_lwip_begin();

                    if (!subscribed) {
                        mqtt_sub_unsub(state->mqtt_client, "pico_w/recv", 0, mqtt_sub_request_cb, 0, 1);
                        subscribed = true;
                    }

                    if (mqtt_test_publish(state) == ERR_OK) {
                        if (state->counter != 0) {
                            DEBUG_printf("published %d\n", state->counter);
                        }
                        timeout = make_timeout_time_ms(5000);
                        state->counter++;
                    } // else ringbuffer is full and we need to wait for messages to flush.
                    cyw43_arch_lwip_end();
                } else {
                    // DEBUG_printf(".");
                }
            }
        }
    }
}

void setup_gpio(); // uses pin defintions and sets address and data bus
int get_requested_address(); // get address from address bus
void put_data_on_bus(uint8_t); // puts data on bus
void setup_cart();  // Will fetch the rom contents

void (*fpBankSwitching)(uint16_t); // function pointer
void BankSwitching_none(uint16_t);
void BankSwitching_F8(uint16_t); // 8K
void BankSwitching_F6(uint16_t); // 16K
void BankSwitching_F4(uint16_t); // 32K


// Variables about the ROM
uint romsize = 4096; // Will be set by setup_cart
uint8_t rom_contents[32768] = { 0 }; // Will be set by setup_cart
uint8_t cart = 0; // Will be set by GPIO pins
uint bankswitch = 0; // Will be set by setup_cart

uint16_t addr;  // address read from address lines
uint16_t romoffset = 0; // offset for bankswitching

uint16_t ce;  // chip-enable value
uint16_t last_ce; // last value to monitor changes in ce 


int main() {

    // Set system clock speed.
    set_sys_clock_khz(250000, true);

    // enable serial
    // stdio_init_all();
    stdio_usb_init();
    sleep_ms(2000); // Wait two seconds to make sure USB-serial is alive

    

    char wifi_pass[] = "X X X X" My wifi has hard to parse chars, so I put it here.

    printf(WIFI_SSID);
    printf("\n");
    // printf(WIFI_PASSWORD);
    // printf(wifi_pass);
    // printf("\n");

    // Initialize the WIFI parts of the PICO_W -- including the internal LED
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    // change wifi_pass to WIFI_PASSWORD
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, wifi_pass, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }

    MQTT_CLIENT_T *state = mqtt_client_init();
     
    run_dns_lookup(state);
 
    mqtt_run_test(state);


    // Turn on the PICO-W led to let us know we have power
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); 

    // A12 Cartridge, Data-ready pin (0 == data on address)
    const uint CE_PIN = 27;
    gpio_init(CE_PIN);
    gpio_set_dir(CE_PIN, GPIO_IN);
    // gpio_pull_up(CE_PIN);

    setup_cart(); // Get rom contents and setup bankswitching
    
    last_ce = 1;
    
    // GPIO setup for address and data bus
    setup_gpio();
    
    // Continually check address lines and
    // put associated data on bus.
    while (true) {

        ce = gpio_get(CE_PIN);
        if (ce != last_ce) {
            last_ce = ce;
            if (last_ce) {
                gpio_set_dir_in_masked(0xFF); // data_bus_input();
            } else{
                gpio_set_dir_out_masked(0xFF); // data_bus_output();
            }
        }
        if (last_ce == 0){
            addr = get_requested_address();
            put_data_on_bus(rom_contents[addr + romoffset]);

            // bankswitching function
            (*fpBankSwitching)(addr);

        }

    }
}

// Function to set up ROM from DIP-switches

void setup_cart() {
    // Read in DIPs to get cart to use
    gpio_init(20);
    gpio_set_dir(20, GPIO_IN);
    if (gpio_get(20)){
        cart+=1;
    } 

    gpio_init(21);
    gpio_set_dir(21, GPIO_IN);
    if (gpio_get(21)){
        cart+=2;
    }

    gpio_init(22);
    gpio_set_dir(22, GPIO_IN);
    if (gpio_get(22)){
        cart+=4;
    }

    // gpio_init(26);
    // gpio_set_dir(26, GPIO_IN);
    // if (gpio_get(26)){
    //     cart+=8;
    // }
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    // const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    if (result  > 100){
        cart+=8;
    }

    // Specify contents of emulated ROM.
    if (cart == 0){
        #include "roms_00.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 1){
        #include "roms_01.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 2){
        #include "roms_02.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 3){
        #include "roms_03.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 4){
        #include "roms_04.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 5){
        #include "roms_05.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 6){
        #include "roms_06.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 7){
        #include "roms_07.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 8){
        #include "roms_08.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 9){
        #include "roms_09.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 10){
        #include "roms_10.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 11){
        #include "roms_11.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 12){
        #include "roms_12.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 13){
        #include "roms_13.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 14){
        #include "roms_14.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    } else if (cart == 15){
        #include "roms_15.h"
        romsize = romsize_in;
        for (int i = 0; i < romsize_in; i++){
            rom_contents[i] = rom_contents_in[i];
        }
    }

    if (romsize == 4096){
        bankswitch = 0; // No bankswitching
        fpBankSwitching = &BankSwitching_none;
    } else if (romsize == 8192){
        bankswitch = 1; // F8 bankswitching
        fpBankSwitching = &BankSwitching_F8;
    } else if (romsize == 16384){
        bankswitch = 2; // F6 bankswitching
        fpBankSwitching = &BankSwitching_F6;
    } else if (romsize == 32768){
        bankswitch = 3; // F4 bankswitching
        fpBankSwitching = &BankSwitching_F4;
    }

}

// Bank Switching Functions 

void BankSwitching_none(uint16_t addr_in) {
    romoffset = 0;
}

void BankSwitching_F8(uint16_t addr_in) {
    if (addr_in == 4088){
        romoffset = 0;
    } else if (addr_in == 4089){
        romoffset = 4096;
    }
}

void BankSwitching_F6(uint16_t addr_in) {
    if ((addr_in >= 4086) && (addr_in <= 4089)){
        romoffset = 4096 * (addr_in - 4086);
    }
}


void BankSwitching_F4(uint16_t addr_in) {

    if ((addr_in >= 4084) && (addr_in <= 4091)){
        romoffset = 4096 * (addr_in - 4084);
    }
}


// GPIO Setup Functions 

void setup_gpio() {
    // Address pins.
    gpio_init(A0);
    gpio_set_dir(A0, GPIO_IN);
    gpio_init(A1);
    gpio_set_dir(A1, GPIO_IN);
    gpio_init(A2); 
    gpio_set_dir(A2, GPIO_IN);
    gpio_init(A3);
    gpio_set_dir(A3, GPIO_IN);
    gpio_init(A4);
    gpio_set_dir(A4, GPIO_IN);
    gpio_init(A5);
    gpio_set_dir(A5, GPIO_IN);
    gpio_init(A6);
    gpio_set_dir(A6, GPIO_IN);
    gpio_init(A7);
    gpio_set_dir(A7, GPIO_IN);
    gpio_init(A8);
    gpio_set_dir(A8, GPIO_IN);
    gpio_init(A9);
    gpio_set_dir(A9, GPIO_IN);
    gpio_init(A10);
    gpio_set_dir(A10, GPIO_IN);
    gpio_init(A11);
    gpio_set_dir(A11, GPIO_IN);

    // Data pins.
    gpio_init(D0);
    gpio_set_dir(D0, GPIO_OUT);
    gpio_init(D1);
    gpio_set_dir(D1, GPIO_OUT);
    gpio_init(D2);
    gpio_set_dir(D2, GPIO_OUT);
    gpio_init(D3);
    gpio_set_dir(D3, GPIO_OUT);
    gpio_init(D4);
    gpio_set_dir(D4, GPIO_OUT);
    gpio_init(D5);
    gpio_set_dir(D5, GPIO_OUT);
    gpio_init(D6);
    gpio_set_dir(D6, GPIO_OUT);
    gpio_init(D7);
    gpio_set_dir(D7, GPIO_OUT);
}

int get_requested_address() {
    // Return only first 12 bits from address bus.
    return ((gpio_get_all() >> 8) & 4095);
}

void put_data_on_bus(uint8_t rom_cont) {
    // 8-bits onto the data bus
    gpio_put_masked(255, rom_cont);
}

