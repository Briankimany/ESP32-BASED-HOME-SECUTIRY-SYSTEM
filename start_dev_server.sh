#!/bin/bash

echo "======================================"
echo "Stopping existing processes on ports"
echo "5173 (frontend) and 8080 (backend)"
echo "======================================"

PORTS=(5173 8080)

for PORT in "${PORTS[@]}"
do
    PIDS=$(lsof -t -i:$PORT)

    if [ -n "$PIDS" ]; then
        echo "Killing processes on port $PORT: $PIDS"
        kill -9 $PIDS
    else
        echo "No process running on port $PORT"
    fi
done

echo ""
echo "======================================"
echo "Starting Backend (Go)"
echo "======================================"

cd backend || { echo "Backend folder not found"; exit 1; }

# Start backend in background
go run main.go &
BACKEND_PID=$!

echo "Backend started with PID $BACKEND_PID"

cd ..

echo ""
echo "======================================"
echo "Starting Frontend (React/Vite)"
echo "======================================"

cd frontend || { echo "Frontend folder not found"; exit 1; }

npm run dev &
FRONTEND_PID=$!

echo "Frontend started with PID $FRONTEND_PID"

cd ..

echo ""
echo "======================================"
echo "Dev environment running"
echo "Backend PID: $BACKEND_PID"
echo "Frontend PID: $FRONTEND_PID"
echo "======================================"

# Keep script alive so Ctrl+C kills both
wait $BACKEND_PID $FRONTEND_PID