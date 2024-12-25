// Vérification périodique des temps en attente
async function checkPending() {
  try {
    const response = await fetch('/pending');
    const data = await response.json();
    const popup = document.getElementById('popup');

    if (data.status === 'pending') {
      popup.classList.add('active'); // Affiche la popup
      document.getElementById('time').textContent = data.time;
    } else {
      popup.classList.remove('active'); // Cache la popup
    }
  } catch (error) {
    console.error('Erreur lors de la vérification des temps en attente :', error);
  }
}

// Charger et afficher le leaderboard
async function loadLeaderboard() {
  try {
    const response = await fetch('/leaderboard.csv');
    const csvText = await response.text();
    const rows = csvText.split('\n').filter(row => row.trim() !== '');

    const tbody = document.getElementById('leaderboard');
    tbody.innerHTML = ''; // Réinitialise le tableau

    rows.forEach(row => {
      const [pseudo, time] = row.split(',');
      if (pseudo && time) {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${pseudo}</td><td>${time}</td>`;
        tbody.appendChild(tr);
      }
    });
  } catch (error) {
    console.error('Erreur lors du chargement du leaderboard :', error);
  }
}

// Ignorer un temps invalide
async function ignoreTime() {
  try {
    const response = await fetch('/ignore-time', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
    });
    const result = await response.json();
    console.log(result.message);
    document.getElementById('popup').classList.remove('active'); // Cache la popup
    loadLeaderboard(); // Recharge le leaderboard
  } catch (error) {
    console.error("Erreur lors de l'ignorance du temps :", error);
  }
}

// Initialisation
window.onload = () => {
  loadLeaderboard(); // Charger les données au chargement
  checkPending(); // Vérifier l'état des données en attente

  // Vérifier les temps en attente toutes les 5 secondes
  setInterval(checkPending, 5000);

  // Bouton Ignorer
  document.getElementById('ignore-button').addEventListener('click', ignoreTime);
};
