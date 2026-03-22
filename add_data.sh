#!/bin/bash

BASE_URL="http://localhost:18080"

echo "====================================================="
echo "🔥 ROZPOCZYNAMY BRUTALNY STRESS-TEST BACKENDU 🔥"
echo "Cel: 5 userów, 20 wpisów na każdego (Razem 100 haseł)"
echo "====================================================="

for u in {1..5}; do
    LOGIN="test_user_$u"
    PASS="potezne_haslo_$u"

    echo ""
    echo "👤 Wjeżdża USER $u: $LOGIN"

    # 1. Rejestracja (wynik wrzucamy do /dev/null żeby nie śmiecić)
    curl -s -X POST "$BASE_URL/register" \
    -H "Content-Type: application/json" \
    -d "{\"login\": \"$LOGIN\", \"password\": \"$PASS\"}" > /dev/null

    # 2. Logowanie
    LOGIN_RESP=$(curl -s -X POST "$BASE_URL/login" \
    -H "Content-Type: application/json" \
    -d "{\"login\": \"$LOGIN\", \"password\": \"$PASS\"}")

    # Wyciągamy token
    TOKEN=$(echo "$LOGIN_RESP" | jq -r '.token // empty')

    if [ -z "$TOKEN" ] || [ "$TOKEN" == "null" ]; then
        echo "❌ Błąd logowania dla $LOGIN! Odpowiedź: $LOGIN_RESP"
        exit 1
    fi

    echo -n "✅ Zalogowano. Pakujemy 20 haseł: "

    # 3. Pętla haseł (20 iteracji)
    for h in {1..20}; do
        SERVICE="Serwis_${u}_${h}"
        V_LOGIN="mail_${u}_${h}@mielarka.pl"
        V_PASS="tajne_haslo_usera_${u}_nr_${h}"

        curl -s -X POST "$BASE_URL/vault" \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "{\"service\": \"$SERVICE\", \"username\": \"$V_LOGIN\", \"password\": \"$V_PASS\"}" > /dev/null

        # Pasek postępu
        echo -n "🟢"
    done
    echo ""

    # 4. Sprawdzenie, czy wszystko weszło
    VAULT_COUNT=$(curl -s -X GET "$BASE_URL/vault" -H "Authorization: Bearer $TOKEN" | jq '.credentials | length')
    echo "🛡️ W sejfie usera $LOGIN znajduje się teraz: $VAULT_COUNT wpisów."

done

echo ""
echo "====================================================="
echo "🎉 STRESS-TEST ZAKOŃCZONY SUKCESEM! BAZA PRZEMIELONA! 🎉"
echo "====================================================="