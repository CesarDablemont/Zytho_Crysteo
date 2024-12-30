// Vérification périodique des temps en attente
// async function checkPending() {
//   try {
//     const response = await fetch('/pending');
//     const data = await response.json();
//     const popup = document.getElementById('popup');

//     if (data.status === 'pending') {
//       popup.classList.add('active');
//       document.getElementById('time').textContent = data.time;
//     } else {
//       popup.classList.remove('active');
//     }
//   } catch (error) {
//     console.error('Erreur lors de la vérification des temps en attente :', error);
//   }
// }
async function checkPending() {
  try {
    const response = await fetch('/pending');
    const data = await response.json();
    const popup = document.getElementById('popup');

    if (data.status === 'pending') {
      popup.classList.add('active');

      // Met à jour le temps
      document.getElementById('time').textContent = data.time;

      // Met à jour la source dynamiquement
      const sourceText = data.source == -1 ? 'Temps du maitre' : `Temps de chinois #${data.source}`;
      document.getElementById('source').textContent = sourceText;
    } else {
      popup.classList.remove('active');
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
    tbody.innerHTML = '';

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
    document.getElementById('popup').classList.remove('active');
    loadLeaderboard();
  } catch (error) {
    console.error("Erreur lors de l'ignorance du temps :", error);
  }
}

// Gestion du pop-up de confirmation
function showConfirmationPopup() {
  document.getElementById('confirm-popup').classList.add('active');
}

function hideConfirmationPopup() {
  document.getElementById('confirm-popup').classList.remove('active');
}

// Gestionnaire d'événements pour confirmation
document.getElementById('ignore-button').addEventListener('click', showConfirmationPopup);
document.getElementById('confirm-delete-button').addEventListener('click', () => {
  hideConfirmationPopup();
  ignoreTime();
});
document.getElementById('cancel-delete-button').addEventListener('click', hideConfirmationPopup);

// Initialisation
window.onload = () => {
  loadLeaderboard();
  checkPending();
  setInterval(checkPending, 1000);
};
