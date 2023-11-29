function update_scores()
{
    fetch('/run/results', {
    method: 'GET',
    headers: {
        'Accept': 'text/plain',
        'Content-Type': 'text/plain'
    },
})
   .then(response => response.text())
   .then(response =>
    {
        document.getElementById("scoretext").innerText = response;
        return response;
    });
   //.then(response => console.log(JSON.stringify(response)));
}

function window_load()
{
    update_scores();
    setInterval(update_scores, 1000);
}

window.onload = window_load;