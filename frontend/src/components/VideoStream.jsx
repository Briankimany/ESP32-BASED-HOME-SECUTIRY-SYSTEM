import React, { useEffect, useState } from "react";

function VideoStream() {
  const [streamError, setStreamError] = useState(false);
  const [isStreaming, setIsStreaming] = useState(false);

  const [imageUrl, setImageUrl] = useState(
    "http://localhost:8080/api/video/stream",
  );

  useEffect(() => {
    const interval = setInterval(() => {
      setImageUrl(`http://localhost:8080/api/video/stream?t=${Date.now()}`);
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="panel">
      <h2>Live Video Feed</h2>

      <div className="panel-content">
        <div className="video-container">
          {streamError ? (
            <div className="video-placeholder">
              <p>Video Stream Unavailable</p>

              <div
                className="status-badge badge-offline"
                style={{ marginTop: "15px" }}
              >
                OFFLINE
              </div>
            </div>
          ) : (
            <>
              <img
                src={imageUrl}
                alt="Security Camera Feed"
                style={{ width: "100%", display: "block" }}
                onLoad={() => {
                  setIsStreaming(true);
                  setStreamError(false);
                }}
                onError={() => {
                  setIsStreaming(false);
                  setStreamError(true);
                }}
              />

              {isStreaming && (
                <div
                  style={{
                    position: "absolute",
                    bottom: "10px",
                    right: "10px",
                  }}
                >
                  <span className="status-badge badge-online">● LIVE</span>
                </div>
              )}
            </>
          )}
        </div>

        <div
          style={{
            marginTop: "15px",
            fontSize: "12px",
            color: "#666",
            textAlign: "center",
          }}
        >
          {isStreaming ? (
            <span className="status-connected">
              ● Streaming from ESP32 Camera
            </span>
          ) : (
            <span className="status-disconnected">
              ● Waiting for video stream
            </span>
          )}
        </div>
      </div>
    </div>
  );
}

export default VideoStream;
