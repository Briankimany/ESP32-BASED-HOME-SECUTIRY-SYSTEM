package database

import (
	"database/sql"

	_ "modernc.org/sqlite"
)

var DB *sql.DB

func InitDB() error {
	var err error

	DB, err = sql.Open("sqlite", "./security.db")
	if err != nil {
		return err
	}

	createSensorTable := `
CREATE TABLE IF NOT EXISTS sensor_readings (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	temperature REAL NOT NULL,
	humidity REAL NOT NULL,
	pir BOOLEAN NOT NULL,
	recorded_at DATETIME NOT NULL
);
`

	_, err = DB.Exec(createSensorTable)
	if err != nil {
		return err
	}

	createAccessTable := `
CREATE TABLE IF NOT EXISTS access_logs (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp DATETIME NOT NULL,
	access_type TEXT NOT NULL,
	input_value TEXT NOT NULL,
	authorized BOOLEAN NOT NULL
);
`

	_, err = DB.Exec(createAccessTable)

	return err

}
