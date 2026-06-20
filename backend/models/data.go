package models

// SensorData represents all sensor readings from the ESP32
type SensorData struct {
	Temperature   float64 `json:"temperature"`
	Humidity      float64 `json:"humidity"`
	PIR           bool    `json:"pir"`
	KeypadCode    string  `json:"keypadCode"`
	RFIDTag       string  `json:"rfidTag"`
	Authorization bool    `json:"authorization"`
}
