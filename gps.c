#include <gps.h>
#include <board.h>

const static char *TAG = "GPS";

esp_err_t td_gps_init(void *ctx){

  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_gps_t *Gps = Board->Gps;

  ESP_LOGI(TAG, "Initializing...");

  if (Gps != NULL) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_OK;
  }

  Gps = malloc(sizeof(td_gps_t));
  if (Gps == NULL) {
    return ESP_ERR_NO_MEM;
  }

  Board->Gps = Gps;

  uart_config_t uart_config = {
    .baud_rate = BOARD_GPS_BAUDRATE,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  int intr_alloc_flags = 0;

  ESP_ERROR_CHECK(uart_driver_install(BOARD_GPS_PORT_NUM, STR_SIZE_LARGE, 0, 20, &Gps->uart_queue, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(BOARD_GPS_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(BOARD_GPS_PORT_NUM, BOARD_GPS_TX_PIN, BOARD_GPS_RX_PIN, -1, -1));
  ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(BOARD_GPS_PORT_NUM, 0x0a, 1, 9, 0, 0));
  ESP_ERROR_CHECK(uart_pattern_queue_reset(BOARD_GPS_PORT_NUM, 20));

  return ESP_OK;
}

#define RX_BUF_SIZE 4096

void td_gps_sendcmd(void *ctx, char *cmd){
  uint16_t checksum = (uint16_t)cmd[1];
  size_t len = (strlen(cmd)+6)*sizeof(char);
  char *buffer = malloc(len);
  uint16_t i = 0;
  if(buffer==NULL){
    return;
  }
  for(i = 2; i< strlen(cmd); i++)
  {
    checksum ^= (uint16_t)(cmd[i]);
  }
  snprintf(buffer, len, "%s*%.02X\r\n", cmd, checksum);
  ESP_LOGW(TAG, "CMD: %s", buffer);

  uart_write_bytes(BOARD_GPS_PORT_NUM, buffer, strlen(buffer));
  free(buffer);
}

void td_gps_task(void *ctx)
{
  assert(ctx!=NULL);
  td_board_t *Board = (td_board_t *)ctx;
  td_gps_t *Gps = Board->Gps;
  uint8_t i = 0;
	uart_event_t event;
	size_t buffered_size;
	uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE);
	CMD_t cmdBuf;
	cmdBuf.command = CMD_NMEA;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

  while(1) {
		//Waiting for UART event.
		if(xQueueReceive(Gps->uart_queue, (void * )&event, portMAX_DELAY)) {
  		bzero(data, RX_BUF_SIZE);
			ESP_LOGI(pcTaskGetName(0), "uart[%d] event:", BOARD_GPS_PORT_NUM);
			switch(event.type) {
				//Event of UART receving data
				/*We'd better handler data event fast, there would be much more data events than
				other types of events. If we take too much time on data event, the queue might
				be full.*/
				case UART_DATA:
					ESP_LOGI(pcTaskGetName(0), "[UART DATA]: %zu", event.size);
          uart_get_buffered_data_len(BOARD_GPS_PORT_NUM, &buffered_size);
          uart_read_bytes(BOARD_GPS_PORT_NUM, data, buffered_size, 100 / portTICK_PERIOD_MS);
          //ESP_LOGI(TAG,"RECEIVED: %s", data);
          ESP_LOG_BUFFER_HEXDUMP(TAG, data, buffered_size, ESP_LOG_INFO);

					break;
				//Event of HW FIFO overflow detected
				case UART_FIFO_OVF:
					ESP_LOGW(pcTaskGetName(0), "hw fifo overflow");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// The ISR has already reset the rx FIFO,
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(BOARD_GPS_PORT_NUM);
					xQueueReset(Gps->uart_queue);
					break;
				//Event of UART ring buffer full
				case UART_BUFFER_FULL:
					ESP_LOGW(pcTaskGetName(0), "ring buffer full");
					// If buffer full happened, you should consider encreasing your buffer size
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(BOARD_GPS_PORT_NUM);
					xQueueReset(Gps->uart_queue);
					break;
				//Event of UART RX break detected
				case UART_BREAK:
					ESP_LOGW(pcTaskGetName(0), "uart rx break");
					break;
				//Event of UART parity check error
				case UART_PARITY_ERR:
					ESP_LOGW(pcTaskGetName(0), "uart parity error");
					break;
				//Event of UART frame error
				case UART_FRAME_ERR:
					ESP_LOGW(pcTaskGetName(0), "uart frame error");
					break;
				//UART_PATTERN_DET
				case UART_PATTERN_DET:
					uart_get_buffered_data_len(BOARD_GPS_PORT_NUM, &buffered_size);
					int pos = uart_pattern_pop_pos(BOARD_GPS_PORT_NUM);
					ESP_LOGI(pcTaskGetName(0), "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
					if (pos == -1) {
						// There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
						// record the position. We should set a larger queue size.
						// As an example, we directly flush the rx buffer here.
						uart_flush_input(BOARD_GPS_PORT_NUM);
					} else {
						uart_read_bytes(BOARD_GPS_PORT_NUM, data, buffered_size, 100 / portTICK_PERIOD_MS);
						ESP_LOGI(pcTaskGetName(0), "read data: %s", data);
						ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), data, buffered_size, ESP_LOG_INFO);
						cmdBuf.length = buffered_size;
						memcpy((char *)cmdBuf.payload, (char *)data, buffered_size);
						cmdBuf.payload[buffered_size] = 0;
						//xQueueSend(xQueueCmd, &cmdBuf, 0);
					}
					break;
				//Others
				default:
					ESP_LOGW(pcTaskGetName(0), "uart event type: %d", event.type);
					break;
			} // end switch
		} // end if
	} // end while

	// never reach here
	free(data);
	data = NULL;
	vTaskDelete(NULL);
}
