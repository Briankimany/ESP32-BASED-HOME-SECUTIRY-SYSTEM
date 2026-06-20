package handlers

import (
	"backend/database"
	"backend/models"
	"encoding/json"
	"io"
	"net/http"
	"sync"
	"time"
)

var (
	latestData = models.SensorData{
		Temperature:   25,
		Humidity:      60,
		PIR:           false,
		KeypadCode:    "",
		RFIDTag:       "",
		Authorization: false,
	}

	dataMutex   = &sync.RWMutex{}
	latestFrame []byte
	frameMutex  = &sync.RWMutex{}

	lastSavedTemp float64
	lastSavedHum  float64
	lastSavedPIR  bool
	lastSavedTime time.Time
)

func abs(x float64) float64 {
	if x < 0 {
		return -x
	}
	return x
}

func GetSensorData(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")

	dataMutex.RLock()
	defer dataMutex.RUnlock()

	json.NewEncoder(w).Encode(latestData)

}

func PostSensorData(w http.ResponseWriter, r *http.Request) {

	var data models.SensorData

	err := json.NewDecoder(r.Body).Decode(&data)
	if err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	dataMutex.Lock()
	latestData = data
	dataMutex.Unlock()

	shouldStore := false

	if abs(data.Temperature-lastSavedTemp) >= 0.5 {
		shouldStore = true
	}

	if abs(data.Humidity-lastSavedHum) >= 2.0 {
		shouldStore = true
	}

	if data.PIR != lastSavedPIR {
		shouldStore = true
	}

	if time.Since(lastSavedTime) >= 5*time.Minute {
		shouldStore = true
	}

	if shouldStore {

		err := database.InsertSensorReading(
			data.Temperature,
			data.Humidity,
			data.PIR,
		)

		if err == nil {
			lastSavedTemp = data.Temperature
			lastSavedHum = data.Humidity
			lastSavedPIR = data.PIR
			lastSavedTime = time.Now()
		}
	}

	w.Header().Set("Content-Type", "application/json")

	json.NewEncoder(w).Encode(map[string]string{
		"status": "data received",
	})

}

func GetVideoStream(w http.ResponseWriter, r *http.Request) {

	frameMutex.RLock()
	defer frameMutex.RUnlock()

	if len(latestFrame) == 0 {
		http.Error(w, "No frame available", http.StatusNotFound)
		return
	}

	w.Header().Set("Content-Type", "image/jpeg")
	w.Header().Set("Cache-Control", "no-store, no-cache, must-revalidate")
	w.Header().Set("Pragma", "no-cache")
	w.Header().Set("Expires", "0")

	w.Write(latestFrame)

}

func PostVideoFrame(w http.ResponseWriter, r *http.Request) {

	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	err := r.ParseMultipartForm(10 << 20)
	if err != nil {
		http.Error(w, "Could not parse multipart form", http.StatusBadRequest)
		return
	}

	file, header, err := r.FormFile("frame")
	if err != nil {
		http.Error(w, "Missing frame file", http.StatusBadRequest)
		return
	}
	defer file.Close()

	frameBytes, err := io.ReadAll(file)
	if err != nil {
		http.Error(w, "Could not read frame", http.StatusInternalServerError)
		return
	}

	frameMutex.Lock()
	latestFrame = frameBytes
	frameMutex.Unlock()

	w.Header().Set("Content-Type", "application/json")

	json.NewEncoder(w).Encode(map[string]interface{}{
		"status":   "frame received",
		"filename": header.Filename,
		"size":     len(frameBytes),
	})

}
