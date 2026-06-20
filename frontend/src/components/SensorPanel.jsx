import React from "react";

function SensorPanel({ temperature, humidity, pir }) {
  const getTempColor = (temp) => {
    if (temp > 35) return "high";
    if (temp > 28) return "medium";
    return "low";
  };

  const tempPercent = Math.min((temperature / 50) * 100, 100);

  const humidityPercent = Math.min(humidity, 100);

  return (
    <div className="panel">
      <h2>Environmental Sensors</h2>
      <div className="panel-content">
        <div className="sensor-reading">
          <span className="sensor-label">Temperature</span>
          <span className="sensor-value">
            {temperature.toFixed(1)} <span className="sensor-unit">°C</span>
          </span>
        </div>
        <div className="progress-bar">
          <div
            className={`progress-fill ${getTempColor(temperature)}`}
            style={{ width: `${tempPercent}%` }}
          ></div>
        </div>

        <div className="sensor-reading">
          <span className="sensor-label">Humidity</span>
          <span className="sensor-value">
            {humidity.toFixed(1)} <span className="sensor-unit">%</span>
          </span>
        </div>
        <div className="progress-bar">
          <div
            className="progress-fill"
            style={{ width: `${humidityPercent}%` }}
          ></div>
        </div>

        <div className="sensor-reading">
          <span className="sensor-label">Motion Detection</span>
          <span className="sensor-value">
            <span
              className={`status-indicator ${pir ? "status-active" : "status-inactive"}`}
            ></span>
            {pir ? "Motion Detected" : "No Motion"}
          </span>
        </div>

        {temperature > 35 && (
          <div className="warning">
            Warning: High temperature detected! Risk of overheating.
          </div>
        )}

        {pir && (
          <div className="warning">
            Alert: Motion detected in restricted area!
          </div>
        )}
      </div>
    </div>
  );
}

export default SensorPanel;
