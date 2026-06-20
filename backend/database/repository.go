package database

import "time"

func InsertSensorReading(
	temperature float64,
	humidity float64,
	pir bool,
) error {

	_, err := DB.Exec(
		`INSERT INTO sensor_readings
	(temperature, humidity, pir, recorded_at)
	VALUES (?, ?, ?, ?)`,
		temperature,
		humidity,
		pir,
		time.Now(),
	)

	return err

}

func InsertAccessLog(
	accessType string,
	inputValue string,
	authorized bool,
) error {

	_, err := DB.Exec(
		`INSERT INTO access_logs
	(timestamp, access_type, input_value, authorized)
	VALUES (?, ?, ?, ?)`,
		time.Now(),
		accessType,
		inputValue,
		authorized,
	)

	return err

}
