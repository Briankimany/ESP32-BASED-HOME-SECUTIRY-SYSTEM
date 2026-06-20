package handlers

import (
	"backend/database"
	"encoding/json"
	"net/http"
	"sync"
	"time"
)

type AccessAttempt struct {
	Timestamp  time.Time `json:"timestamp"`
	Type       string    `json:"type"`
	InputValue string    `json:"inputValue"`
	Authorized bool      `json:"authorized"`
}

// memory cache (for fast dashboard UI)
var (
	accessLogs = []AccessAttempt{}
	logMutex   = &sync.RWMutex{}
)

var (
	correctKeypadCode = "1234"
	correctRFIDTag    = "A1B2C3D4"
)

type VerifyKeypadRequest struct {
	Code string `json:"code"`
	Type string `json:"type"`
}

type VerifyRFIDRequest struct {
	Tag  string `json:"tag"`
	Type string `json:"type"`
}

type VerifyResponse struct {
	Authorized bool   `json:"authorized"`
	Message    string `json:"message"`
	Timestamp  string `json:"timestamp"`
}

func VerifyKeypadCode(w http.ResponseWriter, r *http.Request) {

	var req VerifyKeypadRequest

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	authorized := req.Code == correctKeypadCode

	response := VerifyResponse{
		Authorized: authorized,
		Timestamp:  time.Now().Format(time.RFC3339),
	}

	if authorized {
		response.Message = "Access granted - Door unlocked"
	} else {
		response.Message = "Access denied - Invalid code"
	}

	logMutex.Lock()
	accessLogs = append(accessLogs, AccessAttempt{
		Timestamp:  time.Now(),
		Type:       "keypad",
		InputValue: req.Code,
		Authorized: authorized,
	})
	if len(accessLogs) > 50 {
		accessLogs = accessLogs[len(accessLogs)-50:]
	}
	logMutex.Unlock()

	_ = database.InsertAccessLog(
		"keypad",
		req.Code,
		authorized,
	)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)

}

func VerifyRFIDTag(w http.ResponseWriter, r *http.Request) {
	var req VerifyRFIDRequest

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	authorized := req.Tag == correctRFIDTag

	response := VerifyResponse{
		Authorized: authorized,
		Timestamp:  time.Now().Format(time.RFC3339),
	}

	if authorized {
		response.Message = "Access granted - Welcome"
	} else {
		response.Message = "Access denied - Invalid RFID tag"
	}

	logMutex.Lock()
	accessLogs = append(accessLogs, AccessAttempt{
		Timestamp:  time.Now(),
		Type:       "rfid",
		InputValue: req.Tag,
		Authorized: authorized,
	})
	if len(accessLogs) > 50 {
		accessLogs = accessLogs[len(accessLogs)-50:]
	}
	logMutex.Unlock()

	_ = database.InsertAccessLog(
		"rfid",
		req.Tag,
		authorized,
	)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)

}

func GetAccessLogs(w http.ResponseWriter, r *http.Request) {

	logMutex.RLock()
	defer logMutex.RUnlock()

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(accessLogs)

}
