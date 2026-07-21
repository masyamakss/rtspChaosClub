const sourceModeToggle = document.getElementById("sourceModeToggle");
const syntheticSettings = document.getElementById("syntheticSettings");
const fileSettings = document.getElementById("fileSettings");
const startButton = document.getElementById("startButton");
const statusText = document.getElementById("statusText");
const addSourceCard = document.getElementById("addSourceCard");
const sourceCreationPanel = document.getElementById("sourceCreationPanel");
const sourcesContainer = document.getElementById("sourcesContainer");

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
    !(addSourceCard instanceof HTMLButtonElement) ||
    !(sourceCreationPanel instanceof HTMLElement) ||
    !(sourcesContainer instanceof HTMLElement) ||
    !(cubeSpeedInput instanceof HTMLInputElement) ||
    !(backgroundSpeedInput instanceof HTMLInputElement) ||
    !(resolutionSelect instanceof HTMLSelectElement) ||
    !(videoFileInput instanceof HTMLInputElement)
) {
    document.body.textContent = "JS error: page structure is broken";
    throw new Error("Page structure is broken");
}

const defaultStartButtonText = startButton.textContent;

/*
 * Позже здесь будем хранить таймеры до тех пор,
 * пока StreamController не подтвердит создание.
 */
const creationTimeouts = new Map();
let nextRequestId = 1;

function updateSourceModeView() {
    const fileModeEnabled = sourceModeToggle.checked;

    syntheticSettings.classList.toggle("hidden", fileModeEnabled);
    fileSettings.classList.toggle("hidden", !fileModeEnabled);
}

function openSourceCreationPanel() {
    addSourceCard.classList.add("hidden");
    sourceCreationPanel.classList.remove("hidden");

    statusText.textContent = "Status: configure source";
}

function closeSourceCreationPanel() {
    sourceCreationPanel.classList.add("hidden");
    addSourceCard.classList.remove("hidden");
}

function setCreationLoading(isLoading) {
    startButton.disabled = isLoading;
    startButton.classList.toggle("is-loading", isLoading);

    if (isLoading) {
        startButton.textContent = "Создание...";
    } else {
        startButton.textContent = defaultStartButtonText;
    }
}

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

function createSourceInfoRow(labelText, valueText) {
    const row = document.createElement("div");
    row.className = "source-card-row";

    const label = document.createElement("span");
    label.className = "source-card-label";
    label.textContent = labelText;

    const value = document.createElement("span");
    value.className = "source-card-value";
    value.textContent = valueText;

    row.append(label, value);

    return row;
}

function setSourceCardState(card, state) {
    card.dataset.state = state;

    const stateElement = card.querySelector(".source-state");

    if (!(stateElement instanceof HTMLElement)) {
        console.error("Source card state element was not found");
        return;
    }

    stateElement.textContent = state;

    card.classList.remove(
        "source-card-creating",
        "source-card-created",
        "source-card-error",
        "source-card-running",
        "source-card-stopped"
    );

    card.classList.add(
        "source-card-" + state.toLowerCase()
    );
}

function createSourceCard(requestId, settings) {
    const card = document.createElement("article");

    card.className = "source-card source-card-creating";
    card.dataset.requestId = String(requestId);
    card.dataset.state = "CREATING";

    const header = document.createElement("div");
    header.className = "source-card-header";

    const title = document.createElement("h3");
    title.className = "source-card-title";
    title.textContent = "Источник #" + requestId;

    const state = document.createElement("span");
    state.className = "source-state";
    state.textContent = "CREATING";

    header.append(title, state);

    const body = document.createElement("div");
    body.className = "source-card-body";

    body.append(
        createSourceInfoRow(
            "Тип",
            settings.mode === "synthMode"
                ? "Generated video"
                : "File video"
        )
    );

    if (settings.mode === "synthMode") {
        body.append(
            createSourceInfoRow("Разрешение", settings.resolution),
            createSourceInfoRow(
                "Скорость куба",
                String(settings.cubeSpeed)
            ),
            createSourceInfoRow(
                "Скорость фона",
                String(settings.backgroundSpeed)
            )
        );
    } else {
        body.append(
            createSourceInfoRow("Файл", settings.fileName)
        );
    }

    const actions = document.createElement("div");
    actions.className = "source-card-actions";

    const runSourceButton = document.createElement("button");
    runSourceButton.className = "source-start-button";
    runSourceButton.textContent = "Запустить";
    runSourceButton.disabled = true;

    const deleteSourceButton = document.createElement("button");
    deleteSourceButton.className = "source-delete-button";
    deleteSourceButton.textContent = "Удалить";
    deleteSourceButton.disabled = true;
    deleteSourceButton.addEventListener("click", ()=>
        {
            const streamId = Number(card.dataset.streamId);

            if (!Number.isInteger(streamId)) 
            {
                console.error("Card has no valid streamId");
                return;
            }

            console.log("Delete source:", streamId);
        });

    actions.append(runSourceButton, deleteSourceButton);

    const footer = document.createElement("div");
    footer.className = "source-card-footer";
    footer.textContent = "Ожидание ответа от StreamController";

    card.append(header, body, footer, actions);

    sourcesContainer.insertBefore(card, addSourceCard);

    const timeoutId = window.setTimeout(() => {
        if (card.dataset.state !== "CREATING") {
            return;
        }

        setSourceCardState(card, "ERROR");
        footer.textContent =
            "StreamController не подтвердил создание за 15 секунд";

        statusText.textContent =
            "Status: source request #" +
            requestId +
            " creation timed out";

        creationTimeouts.delete(requestId);
    }, 15000);

    creationTimeouts.set(requestId, timeoutId);

    return card;
}

async function compileSettingsAndSendToStart() {
    const result = collectStartSettings();

    if (!result.ok) {
        statusText.textContent = "Status: " + result.error;
        return;
    }

    const settings = result.settings;
    const requestId = nextRequestId++;

    /*
     * Карточка появляется до отправки команды.
     * Поэтому SSE уже сможет найти её по requestId,
     * даже если StreamController ответит очень быстро.
     */
    const card = createSourceCard(requestId, settings);

    closeSourceCreationPanel();
    setCreationLoading(true);

    statusText.textContent =
        "Status: sending request #" + requestId;

    const markRequestFailed = message => {
        /*
         * SSE мог уже успешно перевести карточку в CREATED.
         * В таком случае поздняя HTTP-ошибка не должна
         * перезаписать её состояние.
         */
        if (card.dataset.state !== "CREATING") {
            return;
        }

        const timeoutId = creationTimeouts.get(requestId);

        if (timeoutId !== undefined) {
            clearTimeout(timeoutId);
            creationTimeouts.delete(requestId);
        }

        setSourceCardState(card, "ERROR");

        const footer = card.querySelector(".source-card-footer");

        if (footer instanceof HTMLElement) {
            footer.textContent = message;
        }
    };

    try {
        const response = await fetch("/api/test", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                requestId: requestId,
                ...settings
            })
        });

        const responseText = await response.text();

        if (!response.ok) {
            markRequestFailed(
                "Server error " +
                response.status +
                ": " +
                responseText
            );

            statusText.textContent =
                "Status: server error " +
                response.status +
                ": " +
                responseText;

            return;
        }

        let responseData;

        try {
            responseData = JSON.parse(responseText);
        } catch (error) {
            markRequestFailed("Server returned invalid JSON");

            statusText.textContent =
                "Status: server returned invalid JSON";

            console.error(
                "Invalid server response:",
                responseText
            );

            return;
        }

        if (responseData.accepted !== true) {
            markRequestFailed("Server rejected the request");

            statusText.textContent =
                "Status: incorrect response from server";

            console.error(
                "Incorrect response:",
                responseData
            );

            return;
        }

        const runSourceButton =
        card.querySelector(".source-start-button");

        const deleteSourceButton =
            card.querySelector(".source-delete-button");

        if (runSourceButton instanceof HTMLButtonElement) {
            runSourceButton.disabled = false;
        }

        if (deleteSourceButton instanceof HTMLButtonElement) {
            deleteSourceButton.disabled = false;
        }

        /*
         * Карточка здесь уже существует.
         * Она могла даже успеть перейти в CREATED через SSE.
         */
        if (card.dataset.state === "CREATING") {
            statusText.textContent =
                "Status: request #" +
                requestId +
                " accepted";
        }
    } catch (error) {
        markRequestFailed("Failed to send creation request");

        statusText.textContent =
            "Status: request failed";

        console.error(
            "Failed to create source:",
            error
        );
    } finally {
        setCreationLoading(false);
    }
}

const sourceEvents = new EventSource("/api/events");

sourceEvents.addEventListener("source-created", event => {
    const data = JSON.parse(event.data);

    statusText.textContent = "Status: SOURCE-CREATEd";
    const card = document.querySelector(
        `[data-request-id="${data.requestId}"]`
    );

    if (!(card instanceof HTMLElement)) {
        console.error("Source card was not found");
        return;
    }

    const timeoutId = creationTimeouts.get(data.requestId);

    if (timeoutId !== undefined) {
        clearTimeout(timeoutId);
        creationTimeouts.delete(data.requestId);
    }

    card.dataset.streamId = String(data.streamId);
    setSourceCardState(card, "CREATED");

    const footer = card.querySelector(".source-card-footer");

    if (footer instanceof HTMLElement) {
        footer.textContent = "RTSP mount point: " + data.mountPoint;
    }
});

sourceEvents.addEventListener("source-creation-failed", event => {
    // найти карточку по requestId
    // отменить таймер
    // поставить ERROR
    // показать reason
});

addSourceCard.addEventListener("click", openSourceCreationPanel);
sourceModeToggle.addEventListener("change", updateSourceModeView);
startButton.addEventListener("click", compileSettingsAndSendToStart);

updateSourceModeView();