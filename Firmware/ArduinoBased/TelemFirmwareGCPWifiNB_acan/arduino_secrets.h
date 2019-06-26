
// wifi info
#define SECRET_SSID "WaiveCar"
#define SECRET_PASS "Waivecar1547"

// NB settings
#define SECRET_PINNUMBER     ""

// Fill in your Google Cloud Platform - IoT Core info
#define SECRET_PROJECT_ID   "mkr-gcp-tutorial-236417"
#define SECRET_CLOUD_REGION "us-central1"
#define SECRET_REGISTRY_ID  "MKR_GCP_Tutorial"
#ifdef ARDUINO_SAMD_MKR1000
#define SECRET_DEVICE_ID    "MyMKR1000"
#elif defined(ARDUINO_SAMD_MKRNB1500)
#define SECRET_DEVICE_ID    "MyMKRNB1500"
#endif
