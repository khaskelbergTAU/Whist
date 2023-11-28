function update_scores()
{
    fetch('/scores', {
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

window.onload = update_scores;