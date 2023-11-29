function update_scores() {
  try {
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
      .then((response) => response.text())
      .then((response) => {
        document.getElementById("scoretext").innerText = response;
        return response;
      });
  } catch (err) {}
  //.then(response => console.log(JSON.stringify(response)));
  try {
    fetch("/run/errors", {
      method: "GET",
      headers: {
        Accept: "text/plain",
        "Content-Type": "text/plain",
      },
    })
      .then((response) => response.text())
      .then((response) => {
        document.getElementById("errortext").innerText = response;
        return response;
      });
  } catch (err) {}
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
