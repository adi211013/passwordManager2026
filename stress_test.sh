#!/bin/bash

BASE_URL="http://localhost:18080"
NUM_USERS=200
ENTRIES_PER_USER=50

# Obliczamy łączną liczbę requestów (Rejestracja + Logowanie + N*Dodanie + Pobranie)
TOTAL_REQ=$(( NUM_USERS * (3 + ENTRIES_PER_USER) ))

echo "======================================================"
echo "💀 OSTATECZNY STRESS TEST (Wydanie: Stopienie Rdzenia) 💀"
echo "👥 Userzy: $NUM_USERS"
echo "🔑 Hasła na usera: $ENTRIES_PER_USER"
echo "🌐 Łączna liczba requestów: $TOTAL_REQ"
echo "======================================================"
echo "Przygotuj gaśnicę. START..."

# Uruchamiamy stoper
START_TIME=$SECONDS

simulate_user() {
    local id=$1
    local login="heavy_user_${id}"
    local pass="PotezneHaslo123!"

    # 1. Rejestracja (Ciężkie hashowanie Argon2id)
    curl -s -X POST "$BASE_URL/register" \
         -H "Content-Type: application/json" \
         -d "{\"login\":\"$login\",\"password\":\"$pass\"}" > /dev/null

    # 2. Logowanie
    local login_resp=$(curl -s -X POST "$BASE_URL/login" \
         -H "Content-Type: application/json" \
         -d "{\"login\":\"$login\",\"password\":\"$pass\"}")

    local token=$(echo "$login_resp" | jq -r '.token')

    if [ "$token" == "null" ] || [ -z "$token" ]; then
        echo "❌ Błąd dla $login! Serwer padł lub odrzucił połączenie."
        return
    fi

    # 3. Masowe dodawanie haseł
    for i in $(seq 1 $ENTRIES_PER_USER); do
        curl -s -X POST "$BASE_URL/vault" \
             -H "Authorization: Bearer $token" \
             -H "Content-Type: application/json" \
             -d "{\"service\":\"Serwis_${i}\",\"username\":\"heavy_${id}\",\"password\":\"tajne_${i}\"}" > /dev/null
    done

    # 4. Pobieranie całego sejfu (Test deszyfrowania i przesyłu)
    curl -s -X GET "$BASE_URL/vault" \
         -H "Authorization: Bearer $token" > /dev/null

    # Żeby nie zablokować konsoli tysiącami logów, wypisujemy tylko co dziesiątego usera
    if (( id % 10 == 0 )); then
        echo "✅ User $id zakończył pełen cykl."
    fi
}

# ==========================================
# ATAK KLASTEROWY
# ==========================================
for u in $(seq 1 $NUM_USERS); do
    simulate_user $u &
done

wait # Czekamy aż ostatni curl umrze lub wróci z tarczą

# Zatrzymujemy stoper
ELAPSED_TIME=$(($SECONDS - $START_TIME))

echo "======================================================"
echo "🏁 TEST ZAKOŃCZONY!"
echo "⏱️ Czas trwania: $ELAPSED_TIME sekund."
echo "🚀 Średnia przepustowość: $(( TOTAL_REQ / (ELAPSED_TIME == 0 ? 1 : ELAPSED_TIME) )) requestów na sekundę."
echo "======================================================"