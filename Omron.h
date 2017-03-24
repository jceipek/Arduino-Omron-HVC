#ifndef _OMRON
#define _OMRON

#include "Arduino.h"
#include <stdint.h>

typedef enum {
  O_GET_MODEL_AND_VERSION = 0x00,
  O_SET_CAMERA_ANGLE = 0x01,
  O_GET_CAMERA_ANGLE = 0x02,
  /* Reserved */
  O_EXECUTE_DETECTION = 0X04,
  O_SET_THRESHOLD_VALUE = 0X05,
  O_GET_THRESHOLD_VALUE = 0X06,
  O_SET_DETECTION_SIZE = 0X07,
  O_GET_DETECTION_SIZE = 0X08,
  O_SET_FACE_ANGLE = 0X09,
  O_GET_FACE_ANGLE = 0X0A,
  /* Reserved */
  O_SET_UART_FORWARDING_RATE = 0X0E,
  /* Reserved */
  O_REGISTER_DATA = 0X10,
  O_DELETE_SPECIFIED_DATA = 0X11,
  O_DELETE_SPECIFIED_USER = 0X12,
  O_DELETE_ALL_DATA = 0X13,
  /* Reserved */
  O_GET_USER_INFO = 0X15,
  /* Reserved */
  O_SAVE_ALBUM = 0X20,
  O_LOAD_ALBUM = 0X21,
  O_SAVE_ALBUM_ON_FLASH_ROM = 0X22,
  /* Reserved */
  O_REFORMAT_FLASH_ROM = 0X30
} OMRON_COMMAND_CODES;

typedef enum {
  A_0   = 0x00,
  A_90  = 0x01,
  A_180 = 0x02,
  A_270 = 0x03
} OMRON_CAMERA_ANGLE;

typedef enum {
  R_9600   = 0x00,
  R_38400  = 0x01,
  R_115200 = 0x02,
  R_230400 = 0x03,
  R_460800 = 0x04,
  R_921600 = 0x05
} OMRON_FORWARDING_RATE;


typedef enum {
  D_HUMAN_BODY_DETECTION = 1 << 0,
  D_HAND_DETECTION = 1 << 1,
  D_FACE_DETECTION = 1 << 2 ,
  D_FACE_DIRECTION_ESTIMATION = 1 << 3,
  D_AGE_ESTIMATION = 1 << 4,
  D_GENDER_ESTIMATION = 1 << 5,
  D_GAZE_ESTIMATION = 1 << 6,
  D_BLINK_ESTIMATION = 1 << 7,
  D_EXPRESSION_ESTIMATION = 1 << 8,
  D_FACE_RECOGNITION = 1 << 9
} DETECTION_FLAGS;

struct human_body_detection_result
{

};
typedef struct human_body_detection_result HumanBodyDetectionResult;

struct hand_detection_result
{

};
typedef struct hand_detection_result HandDetectionResult;

struct face_detection_result
{

};
typedef struct face_detection_result FaceDetectionResult;

struct face_direction_estimation_result
{

};
typedef struct face_direction_estimation_result FaceDirectionEstimationResult;

struct age_estimation_result
{
  int8_t age;
  int16_t confidence;
};
typedef struct age_estimation_result AgeEstimationResult;

struct gender_estimation_result
{
  int8_t gender;
  int16_t confidence;
};
typedef struct gender_estimation_result GenderEstimationResult;

struct gaze_estimation_result
{
  signed char yaw_angle;
  signed char pitch_angle;
};
typedef struct gaze_estimation_result GazeEstimationResult;

struct blink_estimation_result
{

};
typedef struct blink_estimation_result BlinkEstimationResult;

struct expression_estimation_result
{
  int8_t neutral;
  int8_t happiness;
  int8_t surprise;
  int8_t anger;
  int8_t sadness;
  int8_t expression_degree;
};
typedef struct expression_estimation_result ExpressionEstimationResult;

struct face_recognition_result
{
  int16_t user_id;
  int16_t score;
};
typedef struct face_recognition_result FaceRecognitionResult;

struct face_detection_related_result
{
  FaceDetectionResult face_detection;
  FaceDirectionEstimationResult face_direction_estimation;
  AgeEstimationResult age_estimation;
  GenderEstimationResult gender_estimation;
  GazeEstimationResult gaze_estimation;
  BlinkEstimationResult blink_estimation;
  ExpressionEstimationResult expression_estimation;
  FaceRecognitionResult face_recognition;
};
typedef struct face_detection_related_result FaceDetectionRelatedResult;

struct omron
{
  Stream* stream;
  uint8_t read_buffer[1894];
  HumanBodyDetectionResult human_body_detection_results[35];
  uint8_t human_body_detection_result_count;
  HandDetectionResult hand_detection_results[35];
  uint8_t hand_detection_result_count;
  FaceDetectionRelatedResult face_detection_related_results[35];
  uint8_t face_detection_related_result_count;
};
typedef struct omron Omron;

int Get_int_from_bytes (uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return (int)(a |
         b << 8 |
         c << 16 |
         d << 24);
}

int16_t Get_short_from_bytes (uint8_t a, uint8_t b) {
  return (int16_t)(a | b << 8);
}

void Omron_send (Omron* omron, uint8_t commandCode, uint8_t* data, short datalength) {
  omron->stream->write(0xFE); // Synchronous code
  omron->stream->write(commandCode);
  uint8_t msb = datalength >> 8;
  uint8_t lsb = datalength & 0xFF;
  omron->stream->write(lsb);
  omron->stream->write(msb);
  if (datalength > 0) {
    omron->stream->write(data, datalength);
  }
  omron->stream->flush();
}

int Omron_read_non_blocking (Omron* omron) {
  if (omron->stream->available() == 0) {
    return -1;
  }
  uint8_t sync_code = omron->stream->read();
  if (sync_code != 0xFE) {
    return -2;
  }
  uint8_t response_code = omron->stream->read();
  if (response_code != 0x00) {
    return -1000 - response_code;
  }
  uint8_t l_lsb = omron->stream->read();
  uint8_t l_msb = omron->stream->read();
  uint8_t h_lsb = omron->stream->read();
  uint8_t h_msb = omron->stream->read();
  int datalength = Get_int_from_bytes(l_lsb, l_msb, h_lsb, h_msb);
  for (int i = 0; i < datalength; i++) {
    omron->read_buffer[i] = omron->stream->read();
  }
  return datalength;
}

void Omron_wait (Omron* omron) {
  while (omron->stream->available() == 0) {
    delay(1);
  }
}

int Omron_read (Omron* omron) {
  Omron_wait(omron);
  uint8_t sync_code = omron->stream->read();
  if (sync_code != 0xFE) {
    return -2;
  }
  Omron_wait(omron);
  uint8_t response_code = omron->stream->read();
  if (response_code != 0x00) {
    return -1000 - response_code;
  }
  Omron_wait(omron);
  uint8_t l_lsb = omron->stream->read();
  Omron_wait(omron);
  uint8_t l_msb = omron->stream->read();
  Omron_wait(omron);
  uint8_t h_lsb = omron->stream->read();
  Omron_wait(omron);
  uint8_t h_msb = omron->stream->read();
  int datalength = Get_int_from_bytes(l_lsb, l_msb, h_lsb, h_msb);
  for (int i = 0; i < datalength; i++) {
    Omron_wait(omron);
    omron->read_buffer[i] = omron->stream->read();
  }
  return datalength;
}

void Omron_setup (Omron* omron, HardwareSerial* serial) {
  // serial->begin(115200);
  serial->begin(9600);
  omron->stream = serial;
}

int Omron_send_set_UART_forwarding_rate (Omron* omron, OMRON_FORWARDING_RATE rate) {
  uint8_t ratebuf[] = { rate };
  Omron_send(omron, O_SET_UART_FORWARDING_RATE, ratebuf, sizeof(ratebuf));
  // TODO(JULIAN): Need to get response at new rate?
  return Omron_read(omron);
}

int Omron_change_speed (Omron* omron, OMRON_FORWARDING_RATE rate) {
  HardwareSerial* serial = (HardwareSerial*)(omron->stream);
  int response_code = Omron_send_set_UART_forwarding_rate(omron, rate);
  if (response_code < 0) {
    return response_code;
  } else {
    serial->flush();
    delay(2);
    serial->end();
    switch (rate) {
      case R_38400:
        serial->begin(38400);
        break;
      case R_115200:
        serial->begin(115200);
        break;
      case R_230400:
        serial->begin(230400);
        break;
      case R_460800:
        serial->begin(460800);
        break;
      case R_921600:
        serial->begin(921600);
        break;
      case R_9600:
      default:
        serial->begin(9600);
        break;
    }
    return 0;
  }
}

int Omron_send_get_model_and_version (Omron* omron) {
  Omron_send(omron, O_GET_MODEL_AND_VERSION, NULL, 0);
  return Omron_read(omron);
}

int Omron_send_set_camera_angle (Omron* omron, OMRON_CAMERA_ANGLE angle) {
  uint8_t anglebuf[] = { angle };
  Omron_send(omron, O_SET_CAMERA_ANGLE, anglebuf, sizeof(anglebuf));
  return Omron_read(omron);
}

void Omron_parse_detection_results (Omron* omron,
                   uint16_t detection_specifier) {
  uint8_t *read_buffer = &omron->read_buffer[0];
  uint8_t num_detected_bodies = *read_buffer; read_buffer++;
  uint8_t num_detected_hands = *read_buffer; read_buffer++;
  omron->face_detection_related_result_count = *read_buffer; read_buffer++;
  read_buffer++; // 4th element is reserved, so skip over it
  if (detection_specifier & D_HUMAN_BODY_DETECTION) {

    read_buffer += num_detected_bodies * 8; //TODO
  }
  if (detection_specifier & D_HAND_DETECTION) {

    read_buffer += num_detected_hands * 8; //TODO
  }

  for (int i = 0; i < omron->face_detection_related_result_count; i++) {
    if (detection_specifier & D_FACE_DETECTION) {
      read_buffer += 8; //TODO
    }
    if (detection_specifier & D_FACE_DIRECTION_ESTIMATION) {
      read_buffer += 8; //TODO
    }
    if (detection_specifier & D_AGE_ESTIMATION) {
      omron->face_detection_related_results[i].age_estimation.age = (int8_t)*read_buffer; read_buffer++;
      uint8_t lsb = *read_buffer; read_buffer++;
      uint8_t msb = *read_buffer; read_buffer++;
      omron->face_detection_related_results[i].age_estimation.confidence = Get_short_from_bytes(lsb, msb);
    }
    if (detection_specifier & D_GENDER_ESTIMATION) {
      omron->face_detection_related_results[i].gender_estimation.gender = (int8_t)*read_buffer; read_buffer++;
      uint8_t lsb = *read_buffer; read_buffer++;
      uint8_t msb = *read_buffer; read_buffer++;
      omron->face_detection_related_results[i].gender_estimation.confidence = Get_short_from_bytes(lsb, msb);
    }
    if (detection_specifier & D_GAZE_ESTIMATION) {
      omron->face_detection_related_results[i].gaze_estimation.yaw_angle = (int8_t)*read_buffer; read_buffer++; // yaw
      omron->face_detection_related_results[i].gaze_estimation.pitch_angle = (int8_t)*read_buffer; read_buffer++; // pitch
    }
    if (detection_specifier & D_BLINK_ESTIMATION) {
      read_buffer += 4; // TODO
    }
    if (detection_specifier & D_EXPRESSION_ESTIMATION) {
      omron->face_detection_related_results[i].expression_estimation.neutral = (int8_t)*read_buffer; read_buffer++;
      omron->face_detection_related_results[i].expression_estimation.happiness = (int8_t)*read_buffer; read_buffer++;
      omron->face_detection_related_results[i].expression_estimation.surprise = (int8_t)*read_buffer; read_buffer++;
      omron->face_detection_related_results[i].expression_estimation.anger = (int8_t)*read_buffer; read_buffer++;
      omron->face_detection_related_results[i].expression_estimation.sadness = (int8_t)*read_buffer; read_buffer++;
      omron->face_detection_related_results[i].expression_estimation.expression_degree = (int8_t)*read_buffer; read_buffer++;
    }
    if (detection_specifier & D_FACE_RECOGNITION) {
      uint8_t lsb, msb;
      lsb = *read_buffer; read_buffer++;
      msb = *read_buffer; read_buffer++;
      omron->face_detection_related_results[i].face_recognition.user_id = Get_short_from_bytes(lsb, msb);
      lsb = *read_buffer; read_buffer++;
      msb = *read_buffer; read_buffer++;
      omron->face_detection_related_results[i].face_recognition.score = Get_short_from_bytes(lsb, msb);
    }
  }
}

int Omron_send_execute_detection (Omron* omron, uint16_t detection_specifier) {
  uint8_t second_detection_specifier = detection_specifier >> 8;
  uint8_t first_detection_specifier = detection_specifier & 0xFF;
  const uint8_t no_image = 0x00; // Note: Image data is currently unsupported by this library
  uint8_t data[] = { first_detection_specifier, second_detection_specifier, no_image };
  Omron_send(omron, O_EXECUTE_DETECTION, data, sizeof(data));
  int read_code = Omron_read(omron);
  if (read_code < 0) {
    return read_code;
  } else {
    Omron_parse_detection_results(omron, detection_specifier);
    return 0; // success
  }
}




typedef enum {
  F_SUCCESS,
  F_RECOGNITION_IMPOSSIBLE,
  F_NO_DATA_IN_ALBUM,
  F_NO_ID
} FACE_RECOGNITION_OUTCOME;

FACE_RECOGNITION_OUTCOME Omron_interpret_face_recognition_result (FaceRecognitionResult* result) {
  if ((uint16_t)(result->user_id) == 0xFF80 && (uint16_t)(result->score) == 0xFF80) {
    // Recognition not possible
    return F_RECOGNITION_IMPOSSIBLE;
  } else if ((uint16_t)(result->user_id) == 0xFF81 && (uint16_t)(result->score) == 0xFF81) {
    // No data was registered in album
    return F_NO_DATA_IN_ALBUM;
  } else if ((uint16_t)(result->user_id) == 0xFFFF) {
    // no corresponding user ID
    return F_NO_ID;
  } else {
    // Success
    return F_SUCCESS;
  }
}

typedef enum {
  G_FEMALE = 0x0,
  G_MALE = 0x1
} GENDER_TYPE;


#endif