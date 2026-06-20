# Smart Security Monitoring System

A full-stack IoT security monitoring system built using ESP32, Go, React, and SQLite.

The system collects environmental and security data from sensors connected to an ESP32 and displays the information through a web dashboard. It also supports RFID and keypad-based access control, camera streaming, and historical data storage.

## Features

- Temperature monitoring
- Humidity monitoring
- Motion detection (PIR sensor)
- RFID authentication
- Keypad authentication
- Access logging
- ESP32 camera frame streaming
- SQLite data persistence
- React dashboard

## Tech Stack

### Backend

- Go
- Chi Router
- SQLite

### Frontend

- React
- Axios

### Hardware

- ESP32
- DHT Sensor
- PIR Sensor
- RFID Reader
- Keypad
- ESP32-CAM

## Project Structure

```text
project-root/
│
├── backend/
│   ├── database/
│   ├── handlers/
│   ├── models/
│   └── main.go
│
├── frontend/
│   ├── src/
│   └── package.json
│
└── README.md
```

---

## Running the Project

### Backend

Navigate to the backend directory:

```bash
cd backend
```

Install dependencies:

```bash
go mod tidy
```

Run the server:

```bash
go run .
```

The backend will start on:

```text
http://localhost:8080
```

---

### Frontend

Navigate to the frontend directory:

```bash
cd frontend
```

Install dependencies:

```bash
npm install
```

Start the development server:

```bash
npm run dev
```

The frontend will be available at:

```text
http://localhost:5173
```

---

## Database

The application uses SQLite for data persistence.

A database file named:

```text
security.db
```

is automatically created when the backend starts.

### Stored Data

#### Sensor Readings

- Temperature
- Humidity
- Motion detection state
- Timestamp

#### Access Logs

- Access type (RFID or keypad)
- Input value
- Authorization status
- Timestamp

---

## Data Storage Strategy

To avoid unnecessary database growth, sensor readings are only stored when:

- Temperature changes by at least 0.5°C
- Humidity changes by at least 2%
- Motion state changes
- Five minutes have elapsed since the previous saved reading

---

## API Endpoints

| Method | Endpoint             | Description             |
| ------ | -------------------- | ----------------------- |
| GET    | `/api/sensors`       | Get current sensor data |
| POST   | `/api/sensors`       | Submit sensor data      |
| POST   | `/api/verify/keypad` | Verify keypad code      |
| POST   | `/api/verify/rfid`   | Verify RFID tag         |
| GET    | `/api/access/logs`   | Get recent access logs  |
| GET    | `/api/video/stream`  | Get latest camera frame |
| POST   | `/api/video/frame`   | Upload camera frame     |

---

## Example Requests

### Verify Keypad

```http
POST /api/verify/keypad
Content-Type: application/json

{
  "code": "1234"
}
```

### Verify RFID

```http
POST /api/verify/rfid
Content-Type: application/json

{
  "tag": "A1B2C3D4"
}
```

### Submit Sensor Data

```http
POST /api/sensors
Content-Type: application/json

{
  "temperature": 26.5,
  "humidity": 65,
  "pir": true
}
```

---

## License

This project is intended for learning and educational purposes.
