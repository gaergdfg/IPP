#!/bin/bash
# jeżeli wynik testu jest niepoprawny pamięć nie jest testowana

if [[ $# != 2 ]]; then
  echo "Sposób uzytkowania: $0 <ścieżka/do/folderu/z/testami> <ścieżka/do/fodleru/z/projektem>." >&2
  exit 1
fi
tests=$(realpath "$1")
project=$(realpath "$2")

if ! [[ -d "$tests" ]]; then
  echo "Podany folder z testami nie istnieje"
  exit 1
fi

if ! [[ -d "$project" ]]; then
  echo "Podany folder z projektem nie istnieje"
  exit 1
fi

gamma_h=$(echo "$project/src/gamma.h" | sed 's;/;\\/;g')

total=0
correct=0
leaked=0

temp_folder=$(mktemp -d)
name="$temp_folder/gamma"

cd "$temp_folder" || exit 1

function traverse_folder() {
  folder="$1"
  shopt -s nullglob
  for f in "$folder"/*.c; do
	run_test "$f"
  done

  shopt -s nullglob
  for d in "$folder"/*/; do
	echo "$d"
	traverse_folder "$(realpath "$d")"
  done
}

function run_test() {
  f="$1"
  ((total++))
  echo -e "\e[1mTest $f \e[0m"
  sed -i "s/#include \"gamma.h\"/#include \"$gamma_h\"/g" "$f"
  cmake -D TEST_FILE="$f" -D CMAKE_BUILD_TYPE="Debug" "$project" >/dev/null
  make >/dev/null
  sed -i "s/#include \"$gamma_h\"/#include \"gamma.h\"/g" "$f"

  time valgrind --error-exitcode=15 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --log-file=/dev/null "$name" <"$f"

  err=$?

  if [[ $err == 15 ]]; then
	echo -e "\e[1;31m\tWyciek pamięci\e[0m"
	((correct++))
	((leaked++))
  elif [[ $err != 0 ]]; then
	echo -e "\e[1;31m\tNiepoprawny wynik\e[0m"
  else
	((correct++))
	echo -e "\e[1;32m\tPoprawny wynik, brak wycieku\e[0m"
  fi

  echo ""
}

traverse_folder "$tests"

echo -e "Poprawne \e[1m$correct\e[0m na \e[1m$total\e[0m testów"

echo -e "Wyciekła pamięć w \e[1m$leaked\e[0m na \e[1m$total\e[0m testów"

if [[ $leaked == 0 ]] && [[ $correct == "$total" ]]; then
  echo -e "\e[1;92mWszystko dobrze! \e[0m"
fi

rm -r "$temp_folder"
