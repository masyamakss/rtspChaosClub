const monitorStatus = document.getElementById("monitorStatus");
const monitorStreams = document.getElementById("monitorStreams");

const eventSource = new EventSource("/api/events");

eventSource.addEventListener("statistics-updated", (event) => {
    const data = JSON.parse(event.data);

    monitorStatus.textContent = `Получено источников: ${data.streams.length}`;

    monitorStreams.textContent = JSON.stringify(data.streams, null, 2);
});