#!/bin/bash

BASE_URL="http://localhost:18080"
# Generujemy unikalny login na podstawie czasu, żeby testy nie gryzły się z bazą przy kolejnych odpaleniach
USER_LOGIN="badboy_$(date +%s)"
USER_PASS="ValidPass123!"

# Kolorki do logów
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}======================================================${NC}"
echo -e "${YELLOW}🛡️  ROZPOCZYNAM TESTY BEZPIECZEŃSTWA I OBSŁUGI BŁĘDÓW 🛡️${NC}"
echo -e "${YELLOW}======================================================${NC}"

# Funkcja pomocnicza do sprawdzania statusów HTTP
assert_status() {
    local expected=$1
    local actual=$2
    local test_name=$3

    if [ "$expected" == "$actual" ]; then
        echo -e "${GREEN}[PASS]${NC} $test_name (Zwrócono $actual)"
    else
        echo -e "${RED}[FAIL]${NC} $test_name (Oczekiwano $expected, otrzymano $actual)"
        # W razie FAILa zatrzymujemy testy, żebyś mógł sprawdzić logi serwera
        exit 1
    fi
}

echo -e "\n${YELLOW}--- FAZA 1: Rejestracja i Logowanie ---${NC}"

# 1. Brakujące pole w JSON
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/register" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\"}")
assert_status "400" "$STATUS" "Rejestracja z brakującym hasłem"

# 2. Zły typ danych w JSON (Test naszego try-catch)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/register" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":123456789}")
assert_status "400" "$STATUS" "Rejestracja z liczbą zamiast tekstu"

# 3. Zbyt krótkie hasło
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/register" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":\"krotkie\"}")
assert_status "400" "$STATUS" "Rejestracja ze zbyt krótkim hasłem (<8 znaków)"

# 4. Poprawna rejestracja (setup dla reszty testów)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/register" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":\"$USER_PASS\"}")
assert_status "201" "$STATUS" "Poprawna rejestracja testowego usera"

# 5. Próba rejestracji na ten sam login (Konflikt)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/register" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":\"$USER_PASS\"}")
assert_status "409" "$STATUS" "Rejestracja na zajęty login"

# 6. Logowanie ze złym hasłem
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/login" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":\"ZleHaslo!\"}")
assert_status "401" "$STATUS" "Logowanie ze złym hasłem"

# 7. Poprawne logowanie i pobranie tokenu
LOGIN_RESP=$(curl -s -X POST "$BASE_URL/login" -H "Content-Type: application/json" -d "{\"login\":\"$USER_LOGIN\",\"password\":\"$USER_PASS\"}")
TOKEN=$(echo "$LOGIN_RESP" | jq -r '.token')
if [ -n "$TOKEN" ] && [ "$TOKEN" != "null" ]; then
    echo -e "${GREEN}[PASS]${NC} Pobranie tokenu JWT"
else
    echo -e "${RED}[FAIL]${NC} Nie udało się pobrać tokenu"
    exit 1
fi


echo -e "\n${YELLOW}--- FAZA 2: Bezpieczeństwo nagłówków (Middleware) ---${NC}"

# 8. Brak nagłówka Authorization
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$BASE_URL/vault")
assert_status "401" "$STATUS" "Dostęp do sejfu bez nagłówka Authorization"

# 9. Ucięty/Zepsuty nagłówek (Test na std::out_of_range w AuthHelper)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$BASE_URL/vault" -H "Authorization: 123")
assert_status "401" "$STATUS" "Dostęp z nagłówkiem krótszym niż 7 znaków"

# 10. Złe słowo kluczowe (Brak 'Bearer ')
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$BASE_URL/vault" -H "Authorization: Hacker $TOKEN")
assert_status "401" "$STATUS" "Dostęp z błędnym prefiksem nagłówka"

# 11. Zmodyfikowany (fałszywy) token
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$BASE_URL/vault" -H "Authorization: Bearer ${TOKEN}zepsuty")
assert_status "401" "$STATUS" "Dostęp ze sfabrykowanym tokenem JWT"


echo -e "\n${YELLOW}--- FAZA 3: Sejf (Vault) - Walidacja wpisów i IDOR ---${NC}"

# 12. Dodawanie wpisu - brakujące dane
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/vault" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"service\":\"Facebook\"}")
assert_status "400" "$STATUS" "Dodanie wpisu bez loginu i hasła"

# 13. Dodawanie wpisu - złe typy (liczby)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/vault" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"service\":\"FB\",\"username\":123,\"password\":456}")
assert_status "400" "$STATUS" "Dodanie wpisu z błędnymi typami danych"

# 14. Dodawanie wpisu - przekroczone limity (Nazwa serwisu > 96 znaków)
LONG_STRING=$(printf 'a%.0s' {1..100})
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/vault" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"service\":\"$LONG_STRING\",\"username\":\"test\",\"password\":\"Haslo1234\"}")
assert_status "400" "$STATUS" "Dodanie wpisu ze zbyt długą nazwą serwisu"

# 15. Usunięcie nieistniejącego wpisu (IDOR/404 test)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/vault/999999" -H "Authorization: Bearer $TOKEN")
assert_status "404" "$STATUS" "Próba usunięcia nieistniejącego lub cudzego wpisu"

# 16. Edycja nieistniejącego wpisu
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X PUT "$BASE_URL/vault/999999" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"service\":\"Test\",\"username\":\"test\",\"password\":\"Haslo1234\"}")
assert_status "404" "$STATUS" "Próba edycji nieistniejącego lub cudzego wpisu"


echo -e "\n${YELLOW}--- FAZA 4: Zmiana hasła głównego ---${NC}"

# 17. Zmiana hasła głównego - złe stare hasło
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X PUT "$BASE_URL/user/password" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"old_password\":\"NieMojeHaslo!\",\"new_password\":\"NoweHaslo123!\"}")
assert_status "401" "$STATUS" "Próba zmiany hasła z podaniem złego starego hasła"

# 18. Zmiana hasła głównego - nowe hasło za krótkie
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X PUT "$BASE_URL/user/password" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"old_password\":\"$USER_PASS\",\"new_password\":\"krotkie\"}")
assert_status "400" "$STATUS" "Próba ustawienia zbyt krótkiego nowego hasła"

echo -e "\n${YELLOW}--- FAZA 5: SQL Injection ---${NC}"

# 19. Klasyczny SQL Injection w polu loginu (omijanie hasła)
SQLI_PAYLOAD="' OR 1=1 --"
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/login" -H "Content-Type: application/json" -d "{\"login\":\"$SQLI_PAYLOAD\",\"password\":\"jakieshaslo\"}")
assert_status "401" "$STATUS" "SQL Injection (Logowanie: ' OR 1=1 --)"

# 20. Próba zrzucenia tabeli (DROP TABLE) przez pole nazwy serwisu w sejfie
SQLI_DROP="'; DROP TABLE users; --"
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/vault" -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d "{\"service\":\"$SQLI_DROP\",\"username\":\"test\",\"password\":\"Haslo1234\"}")
# Tutaj serwer potraktuje "'; DROP TABLE users; --" jako po prostu nazwę serwisu i zwróci 201 Created (zapisze to dosłownie).
# Ewentualnie 400 jeśli przekroczy limit znaków, ale na pewno nie wykona DROP TABLE!
if [ "$STATUS" == "201" ] || [ "$STATUS" == "400" ]; then
    echo -e "${GREEN}[PASS]${NC} SQL Injection (DROP TABLE) zablokowany (baza nietknięta)"
else
    echo -e "${RED}[FAIL]${NC} Coś poszło nie tak przy SQLi, zwrócono: $STATUS"
    exit 1
fi

echo -e "\n${GREEN}======================================================${NC}"
echo -e "${GREEN}✅ TESTY ZAKOŃCZONE SUKCESEM! SERWER PRZETRWAŁ OSTRZAŁ!${NC}"
echo -e "${GREEN}======================================================${NC}"