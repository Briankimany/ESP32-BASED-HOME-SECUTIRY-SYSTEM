import React, { useState, useEffect } from "react";
import SensorPanel from "./components/SensorPanel";
import AuthPanel from "./components/AuthPanel";
import VideoStream from "./components/VideoStream";
import "./styles/App.css";

function App() {
  const [sensorData, setSensorData] = useState({
    temperature: 0,
    humidity: 0,
    pir: false,
    keypadCode: "",
    rfidTag: "",
    authorization: false,
  });

  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [serverStatus, setServerStatus] = useState("checking");
  const [globalAuthStatus, setGlobalAuthStatus] = useState(false);

  const checkServerConnection = async () => {
    try {
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), 3000);

      const response = await fetch(
        "http://air4life.mu.ac.ke:8080/api/sensors",
        {
          signal: controller.signal,
          method: "GET",
          mode: "cors",
        },
      );

      clearTimeout(timeoutId);
      setServerStatus("connected");
      return true;
    } catch (err) {
      console.error("Server connection check failed:", err);
      setServerStatus("disconnected");
      setError(`Cannot connect to backend at http://air4life.mu.ac.ke`);
      setLoading(false);
      return false;
    }
  };

  const fetchSensorData = async () => {
    try {
      const response = await fetch("http://air4life.mu.ac.ke:8080/api/sensors");

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      setSensorData(data);
      setLoading(false);
      setError(null);
      setServerStatus("connected");
    } catch (err) {
      console.error("Error fetching data:", err);
      setError(`Connection error: ${err.message}`);
      setLoading(false);
      setServerStatus("disconnected");
    }
  };

  const handleAuthorizationResult = (isAuthorized) => {
    setGlobalAuthStatus(isAuthorized);
    if (isAuthorized) {
      console.log("Access granted at:", new Date().toLocaleTimeString());
    }
  };

  useEffect(() => {
    checkServerConnection().then((isConnected) => {
      if (isConnected) {
        fetchSensorData();
        const interval = setInterval(fetchSensorData, 2000);
        return () => clearInterval(interval);
      }
    });
  }, []);

  if (loading && serverStatus === "checking") {
    return (
      <div className="App">
        <header>
          <h1>Security Monitoring System</h1>

          <div className="model-info">
            <div className="model">ESP32 Security Monitor</div>
          </div>
        </header>
        <div className="dashboard">
          <div className="loading-container">
            <div className="loading-spinner"></div>
            <p>Connecting to backend server</p>
          </div>
        </div>
      </div>
    );
  }

  if (error || serverStatus === "disconnected") {
    return (
      <div className="App">
        <header>
          <h1>Security Monitoring System</h1>

          <div className="model-info">
            <div className="model">ESP32 Security Monitor</div>
            <div className="version">Connection Error</div>
          </div>
          <div className="connection-status">
            <span className="status-disconnected">● Offline</span>
          </div>
        </header>
        <div className="dashboard">
          <div className="error-container">
            <h2>Connection Error</h2>
            <p>{error}</p>

            <button
              onClick={() => {
                setLoading(true);
                setError(null);
                setServerStatus("checking");
                checkServerConnection().then(() => fetchSensorData());
              }}
            >
              Retry Connection
            </button>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="App">
      <header>
        <h1>Security Monitoring System</h1>

        <div className="model-info">
          <div className="model">ESP32 Security Monitor</div>
        </div>
        <div className="connection-status">
          <span className="status-connected">● Online</span>
          {globalAuthStatus && (
            <span style={{ marginLeft: "15px", color: "#00b894" }}>
              Door Unlocked
            </span>
          )}
        </div>
      </header>

      <div className="dashboard">
        <div className="stats-grid">
          <SensorPanel
            temperature={sensorData.temperature}
            humidity={sensorData.humidity}
            pir={sensorData.pir}
          />

          <AuthPanel onAuthorizationResult={handleAuthorizationResult} />

          <VideoStream />
        </div>
      </div>
    </div>
  );
}

export default App;
