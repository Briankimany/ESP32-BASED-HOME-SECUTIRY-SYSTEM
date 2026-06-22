import { useState } from "react";

function AuthPanel({ onAuthorizationResult }) {
  const [keypadInput, setKeypadInput] = useState("");
  const [rfidInput, setRfidInput] = useState("");
  const [authorizationStatus, setAuthorizationStatus] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [message, setMessage] = useState("");

  const CORRECT_KEYPAD_CODE = import.meta.env.VITE_CORRECT_KEYPAD_CODE;
  const CORRECT_RFID_TAG = import.meta.env.VITE_CORRECT_RFID_TAG;

  const verifyKeypadCode = async () => {
    if (!keypadInput) {
      setMessage("Please enter a code");
      return;
    }

    setIsLoading(true);
    setMessage("");

    try {
      const response = await fetch("/api/verify/keypad", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          code: keypadInput,
          type: "keypad",
        }),
      });

      const result = await response.json();

      if (result.authorized) {
        setAuthorizationStatus(true);
        setMessage("Access Granted! Door Unlocked");
        setKeypadInput("");
      } else {
        setAuthorizationStatus(false);
        setMessage("Access Denied! Invalid Code");
        setKeypadInput("");
      }

      if (onAuthorizationResult) {
        onAuthorizationResult(result.authorized);
      }
    } catch (error) {
      console.error("Error verifying code:", error);
      setMessage("Error connecting to server");
    } finally {
      setIsLoading(false);
    }
  };

  // Function to verify RFID tag with backend
  const verifyRFIDTag = async () => {
    if (!rfidInput) {
      setMessage("Please enter or scan RFID tag");
      return;
    }

    setIsLoading(true);
    setMessage("");

    try {
      // Send to backend for verification
      const response = await fetch("/api/verify/rfid", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          tag: rfidInput,
          type: "rfid",
        }),
      });

      const result = await response.json();

      if (result.authorized) {
        setAuthorizationStatus(true);
        setMessage("Access Granted! Welcome");
        setRfidInput("");
      } else {
        setAuthorizationStatus(false);
        setMessage("Access Denied! Invalid RFID Tag");
        setRfidInput("");
      }

      if (onAuthorizationResult) {
        onAuthorizationResult(result.authorized);
      }
    } catch (error) {
      console.error("Error verifying RFID:", error);
      setMessage("Error connecting to server");
    } finally {
      setIsLoading(false);
    }
  };

  // Handle keypad number button clicks
  const handleKeypadButton = (value) => {
    if (value === "C") {
      setKeypadInput("");
    } else if (value === "⌫") {
      setKeypadInput((prev) => prev.slice(0, -1));
    } else {
      if (keypadInput.length < 6) {
        // Limit to 6 digits
        setKeypadInput((prev) => prev + value);
      }
    }
  };

  return (
    <div className="panel">
      <h2>Access Control System</h2>
      <div className="panel-content">
        {/* Keypad Section */}
        <div style={{ marginBottom: "25px" }}>
          <h3 style={{ fontSize: "16px", marginBottom: "10px", color: "#333" }}>
            Keypad Entry
          </h3>

          <div
            style={{
              background: "#f0f0f0",
              padding: "12px",
              borderRadius: "8px",
              textAlign: "center",
              fontSize: "24px",
              fontFamily: "monospace",
              letterSpacing: "5px",
              marginBottom: "15px",
            }}
          >
            {keypadInput || "____"}
          </div>

          <div
            style={{
              display: "grid",
              gridTemplateColumns: "repeat(3, 1fr)",
              gap: "10px",
              marginBottom: "15px",
            }}
          >
            {["1", "2", "3", "4", "5", "6", "7", "8", "9", "C", "0", "⌫"].map(
              (btn) => (
                <button
                  key={btn}
                  onClick={() => handleKeypadButton(btn)}
                  style={{
                    padding: "15px",
                    fontSize: "18px",
                    fontWeight: "bold",
                    border: "1px solid #ddd",
                    background:
                      btn === "C"
                        ? "#ff7675"
                        : btn === "⌫"
                          ? "#fdcb6e"
                          : "white",
                    borderRadius: "8px",
                    cursor: "pointer",
                    transition: "all 0.2s",
                  }}
                  onMouseEnter={(e) =>
                    (e.target.style.background =
                      btn === "C"
                        ? "#ff5252"
                        : btn === "⌫"
                          ? "#f39c12"
                          : "#f0f0f0")
                  }
                  onMouseLeave={(e) =>
                    (e.target.style.background =
                      btn === "C"
                        ? "#ff7675"
                        : btn === "⌫"
                          ? "#fdcb6e"
                          : "white")
                  }
                >
                  {btn}
                </button>
              ),
            )}
          </div>

          <button
            onClick={verifyKeypadCode}
            disabled={isLoading || !keypadInput}
            style={{
              width: "100%",
              padding: "12px",
              background: "#4a9eff",
              color: "white",
              border: "none",
              borderRadius: "8px",
              fontSize: "16px",
              fontWeight: "bold",
              cursor: keypadInput ? "pointer" : "not-allowed",
              opacity: keypadInput ? 1 : 0.6,
            }}
          >
            {isLoading ? "Verifying..." : "Verify Code"}
          </button>
        </div>

        <div style={{ marginBottom: "25px" }}>
          <h3 style={{ fontSize: "16px", marginBottom: "10px", color: "#333" }}>
            RFID Scanner
          </h3>

          <div className="sensor-reading">
            <span className="sensor-label">RFID Tag</span>
            <input
              type="text"
              value={rfidInput}
              onChange={(e) => setRfidInput(e.target.value.toUpperCase())}
              placeholder="Scan or enter RFID tag"
              style={{
                padding: "10px",
                fontSize: "14px",
                fontFamily: "monospace",
                border: "1px solid #ddd",
                borderRadius: "6px",
                width: "60%",
              }}
            />
          </div>

          <button
            onClick={verifyRFIDTag}
            disabled={isLoading || !rfidInput}
            style={{
              width: "100%",
              padding: "12px",
              background: "#00b894",
              color: "white",
              border: "none",
              borderRadius: "8px",
              fontSize: "16px",
              fontWeight: "bold",
              cursor: rfidInput ? "pointer" : "not-allowed",
              opacity: rfidInput ? 1 : 0.6,
              marginTop: "10px",
            }}
          >
            {isLoading ? "Verifying..." : "Verify RFID Tag"}
          </button>
        </div>

        {message && (
          <div
            className={`authorization-status ${authorizationStatus ? "authorized" : "unauthorized"}`}
            style={{
              padding: "15px",
              borderRadius: "8px",
              textAlign: "center",
              fontWeight: "bold",
              marginTop: "15px",
            }}
          >
            {message}
          </div>
        )}
      </div>
    </div>
  );
}

export default AuthPanel;
