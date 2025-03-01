// Vérification périodique des temps en attente
async function checkPending() {
  try {
    const response = await fetch('/pending');
    const data = await response.json();
    const popup = document.getElementById('popup');

    if (data.status === 'pending') {
      beforeSubmitNewTime();
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

document.getElementById('category').addEventListener('change', function () {
  const customCategoryInput = document.getElementById('custom-category');
  if (this.value === 'autre') {
    customCategoryInput.style.display = 'block';
    customCategoryInput.required = true;
  } else {
    customCategoryInput.style.display = 'none';
    customCategoryInput.required = false;
  }
});

document.getElementById('filter-icon').addEventListener('click', () => {
  document.getElementById('filter-popup').classList.toggle('active');
});

document.getElementById('apply-filters').addEventListener('click', () => {
  applyFiltersAndSort();
  document.getElementById('filter-popup').classList.remove('active');
});

async function applyFiltersAndSort() {
  try {
    const response = await fetch('/leaderboard.csv');
    const csvText = await response.text();
    const rows = csvText.split('\n').filter(row => row.trim() !== '');

    const categoryFilters = document.querySelectorAll('.category-filter:checked');
    const selectedCategories = Array.from(categoryFilters).map(cb => cb.value);

    const sortOption = document.querySelector('input[name="sort"]:checked').value;

    // Filter and sort rows based on selected options
    const filteredRows = rows.filter(row => {
      const [pseudo, time, category] = row.split(',');
      const isStandardCategory = ['25cl', '33cl', '50cl'].includes(category.trim());
      const isOtherCategory = !isStandardCategory && selectedCategories.includes('autre');

      return (isStandardCategory && selectedCategories.includes(category.trim())) || isOtherCategory;
    });

    // Sort rows based on selected option
    filteredRows.sort((a, b) => {
      const [, timeA, categoryA] = a.split(',');
      const [, timeB, categoryB] = b.split(',');

      if (sortOption === 'time-asc') {
        return parseFloat(timeA) - parseFloat(timeB);
      } else if (sortOption === 'time-desc') {
        return parseFloat(timeB) - parseFloat(timeA);
      } else if (sortOption === 'category-asc') {
        return categoryA.localeCompare(categoryB);
      } else if (sortOption === 'category-desc') {
        return categoryB.localeCompare(categoryA);
      }
    });

    // Display filtered and sorted rows
    const tbody = document.getElementById('leaderboard');
    tbody.innerHTML = '';
    filteredRows.forEach(row => {
      const [pseudo, time, category] = row.split(',');
      const tr = document.createElement('tr');
      tr.innerHTML = `<td>${pseudo}</td><td>${time}</td><td>${category}</td>`;
      tbody.appendChild(tr);
    });
  } catch (error) {
    console.error('Erreur lors du chargement et du traitement du leaderboard :', error);
  }
}

// Conserver les sélections de filtres et de tri
function saveFilterAndSortOptions() {
  const categoryFilters = document.querySelectorAll('.category-filter:checked');
  const selectedCategories = Array.from(categoryFilters).map(cb => cb.value);

  const sortOption = document.querySelector('input[name="sort"]:checked').value;

  localStorage.setItem('selectedCategories', JSON.stringify(selectedCategories));
  localStorage.setItem('sortOption', sortOption);
}

function loadFilterAndSortOptions() {
  const selectedCategories = JSON.parse(localStorage.getItem('selectedCategories')) || ['25cl', '33cl', '50cl', 'autre'];
  const sortOption = localStorage.getItem('sortOption') || 'time-asc';

  document.querySelectorAll('.category-filter').forEach(cb => {
    cb.checked = selectedCategories.includes(cb.value);
  });

  document.querySelector(`input[name="sort"][value="${sortOption}"]`).checked = true;
}

// Initialisation
window.onload = () => {
  loadFilterAndSortOptions();
  applyFiltersAndSort();
  checkPending();
  setInterval(checkPending, 1000);
};

// Appeler cette fonction avant de soumettre un nouveau temps
function beforeSubmitNewTime() {
  saveFilterAndSortOptions();
}
