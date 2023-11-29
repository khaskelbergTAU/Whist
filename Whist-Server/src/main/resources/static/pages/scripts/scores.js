function update_scores() {
  path = "/run/results";
  if (current_log < 4) {
    path = "/run/log/" + (current_log + 1);
  }
  fetch(path, {
    method: "GET",
    headers: {
      Accept: "text/plain",
      "Content-Type": "text/plain",
    },
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Error getting scores.");
      }
      return response.text();
    })
    .then((response) => {
      document.getElementById("scoretext").innerText = response;
      return response;
    })
    .catch((error) => {
      document.getElementById("scoretext").innerText = "No Active Runs";
      console.log(error);
    });
  //.then(response => console.log(JSON.stringify(response)));
  fetch("/run/errors", {
    method: "GET",
    headers: {
      Accept: "text/plain",
      "Content-Type": "text/plain",
    },
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Error getting scores.");
      }
      return response.text();
    })
    .then((response) => {
      document.getElementById("errortext").innerText = response;
      return response;
    })
    .catch((error) => {
      document.getElementById("errortext").innerText = "No Active Runs";
      console.log(error);
    });
}

function getLog(n) {
  current_log = n;
  update_scores();
}

function window_load() {
  update_scores();
  setInterval(update_scores, 1000);
}

current_log = 4;
window.onload = window_load;
