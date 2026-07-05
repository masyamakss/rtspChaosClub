const sourceModeToggle = document.getElementById("sourceModeToggle");
const syntheticSettings = document.getElementById("syntheticSettings");
const fileSettings = document.getElementById("fileSettings");
const startButton = document.getElementById("startButton");
const statusText = document.getElementById("statusText");

const cubeSpeedInput = document.getElementById("cubeSpeedInput");
const backgroundSpeedInput = document.getElementById("backgroundSpeedInput");
const resolutionSelect = document.getElementById("resolutionSelect");
const videoFileInput = document.getElementById("videoFileInput");

if (
      !(sourceModeToggle instanceof HTMLInputElement) ||
      !(syntheticSettings instanceof HTMLElement) ||
      !(fileSettings instanceof HTMLElement) ||
      !(startButton instanceof HTMLButtonElement) ||
      !(statusText instanceof HTMLElement) ||
      !(cubeSpeedInput instanceof HTMLInputElement) ||
      !(backgroundSpeedInput instanceof HTMLInputElement) ||
      !(resolutionSelect instanceof HTMLSelectElement) ||
      !(videoFileInput instanceof HTMLInputElement)
) {
      document.body.textContent = "JS error: page structure is broken";
      throw new Error("Page structure is broken");
}

function updateSourceModeView() {
      const fileModeEnabled = sourceModeToggle.checked;

      syntheticSettings.classList.toggle("hidden", fileModeEnabled);
      fileSettings.classList.toggle("hidden", !fileModeEnabled);
}

sourceModeToggle.addEventListener("change", updateSourceModeView);

updateSourceModeView();

function collectStartSettings() {
      if (sourceModeToggle.checked) {
            const selectedFile = videoFileInput.files[0];

            if (!selectedFile) {
                  return {
                        ok: false,
                        error: "Choose video file first"
                  };
            }

            return {
                  ok: true,
                  settings: {
                        mode: "fileMode",
                        fileName: selectedFile.name
                  }
            };
      }

      return {
            ok: true,
            settings: {
                  mode: "synthMode",
                  resolution: resolutionSelect.value,
                  cubeSpeed: Number(cubeSpeedInput.value),
                  backgroundSpeed: Number(backgroundSpeedInput.value)
            }
      };
}

async function compileSettingsAndSendToStart() {
      const result = collectStartSettings();

      if (!result.ok) {
            statusText.textContent = "Status: " + result.error;
            return;
      }

      const settings = result.settings;

      statusText.textContent = "Status: sending " + JSON.stringify(settings);

      try {
            const response = await fetch("/api/test", {
                  method: "POST",
                  headers: {
                        "Content-Type": "application/json"
                  },
                  body: JSON.stringify(settings)
            });

            const responseText = await response.text();

            if (!response.ok) {
                  statusText.textContent =
                        "Status: server error " + response.status + ": " + responseText;
                  return;
            }

            statusText.textContent =
                  "Status: server response: " + responseText;
      } catch (error) {
            statusText.textContent = "Status: request failed";
      }
}

startButton.addEventListener("click", compileSettingsAndSendToStart);