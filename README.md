# Arduino-Omron-HVC
A single-file header-only C-style Arduino library for obtaining face analysis and recognition results from Omron Human Vision Components smart cameras.

**The API is a work in progress and is subject to change**

- Compatible with the [B5T-007001 device](https://plus-sensing.omron.com/product/B5T-007001/)
- Based on the [official command specifications](https://plus-sensing.omron.com/product/B5T-007001/files/commandspecifications/B5T-007001_CommandSpecifications_A.pdf)

## Supported features
### Device
- connect to the device
- get model and version
- set camera angle
### Detection
#### Faces
- number of visible faces
- age estimation
- gender estimation
- gaze direction estimation
- expression estimation (neutral, happy, surprised, angry, sad, degree of positivity)
- face recognition (for faces already stored on the device)

## Usage
1. Drag `Omron.h` into your Arduino project's project folder.
2. Put `#include "Omron.h"` at the top of each file in your project where you want to use the camera.

### Example
```
#define COMPUTER_SERIAL Serial
#include "Omron.h"
Omron _omron;

void setup() {
  COMPUTER_SERIAL.begin(115200);
  delay(10);
  COMPUTER_SERIAL.println("Computer Serial Initialized");

  Omron_setup(&_omron, &Serial1);
  delay(6000);
  COMPUTER_SERIAL.println("Camera Serial Initialized");
  
  // Camera Version
  COMPUTER_SERIAL.write(_omron.read_buffer, Omron_send_get_model_and_version(&_omron));
  
  int response_code = Omron_send_set_camera_angle(&_omron, A_270);
  if (response_code < 0) {
    COMPUTER_SERIAL.println("Set camera angle failed: "+String(response_code));
  } else {
    COMPUTER_SERIAL.println("Camera angle set!");
  }
}

void loop () {
  int detection_code = Omron_send_execute_detection( &_omron,
                                                     D_GENDER_ESTIMATION
                                                   | D_FACE_RECOGNITION
                                                   );
  if (detection_code == 0) {
    for (int i = 0; i < _omron.face_detection_related_result_count; i++) {
      GenderEstimationResult* genderEstimation = &_omron.face_detection_related_results[i].gender_estimation;
      switch ((GENDER_TYPE)(genderEstimation->gender)) {
        case G_FEMALE:
          COMPUTER_SERIAL.print(" Female");
        case G_MALE:
          COMPUTER_SERIAL.print(" Male");
      }

      FaceRecognitionResult *result = &_omron.face_detection_related_results[i];
      switch (Omron_interpret_face_recognition_result(result)) {
        case F_SUCCESS:
          COMPUTER_SERIAL.print(" User ID: ");
          COMPUTER_SERIAL.print(String(result->user_id, DEC));
          COMPUTER_SERIAL.print(" Score: ");
          COMPUTER_SERIAL.println(String(result->score, DEC));
          break;
        case F_RECOGNITION_IMPOSSIBLE:
          COMPUTER_SERIAL.println(" Recognition Impossible");
          break;
        case F_NO_DATA_IN_ALBUM:
          COMPUTER_SERIAL.println(" No Data in Album");
          break;
        case F_NO_ID:
          COMPUTER_SERIAL.println(" Unknown Person");
          break;
      }
    }
  }
}
```

## License
MIT
