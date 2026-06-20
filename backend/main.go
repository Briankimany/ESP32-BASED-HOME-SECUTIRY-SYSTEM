package main

import (
	"backend/database"
	"backend/handlers"
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
)

// CORS middleware
func enableCORS(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		next.ServeHTTP(w, r)
	})

}

func main() {
	err := database.InitDB()
	if err != nil {
		log.Fatal("Failed to initialize database:", err)
	}

	log.Println("Database connected successfully")

	r := chi.NewRouter()

	r.Use(enableCORS)
	r.Use(middleware.Logger)

	r.Get("/health", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte(`{"status":"ok"}`))
	})

	r.Get("/api/sensors", handlers.GetSensorData)
	r.Post("/api/sensors", handlers.PostSensorData)

	r.Post("/api/verify/keypad", handlers.VerifyKeypadCode)
	r.Post("/api/verify/rfid", handlers.VerifyRFIDTag)
	r.Get("/api/access/logs", handlers.GetAccessLogs)

	r.Get("/api/video/stream", handlers.GetVideoStream)
	r.Post("/api/video/frame", handlers.PostVideoFrame)
	port := ":8080"

	log.Println("Server running on http://localhost" + port)

	err = http.ListenAndServe(port, r)
	if err != nil {
		log.Fatal("Server failed to start:", err)
	}

}
