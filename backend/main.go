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
		allowed := map[string]bool{
			"http://localhost:5173":          true,
			"https://air4life.mu.ac.ke:5173": true,
			"http://air4life.mu.ac.ke:5173":  true,
		}

		origin := r.Header.Get("Origin")

		if allowed[origin] {
			w.Header().Set("Access-Control-Allow-Origin", origin)
		}

		// // Always set headers FIRST
		// w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		// Handle preflight request
		if r.Method == http.MethodOptions {
			w.WriteHeader(http.StatusNoContent)
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

	log.Println("Server running on http://air4life.mu.ac.ke" + port)

	err = http.ListenAndServe(port, r)
	if err != nil {
		log.Fatal("Server failed to start:", err)
	}

}
