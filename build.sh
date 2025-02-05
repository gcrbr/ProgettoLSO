printf "=%.0s" $(seq 10)
echo "\nBuilding..."
rm -f run_client run_server
client=$(find client -iname '*.c' | tr -s '\n' ' ' | sed 's/.$//')
server=$(find server -iname '*.c' | tr -s '\n' ' ' | sed 's/.$//')
common=$(find common -iname '*.c' | tr -s '\n' ' ' | sed 's/.$//')
gcc $common $client -o run_client
gcc $common $server -o run_server
locs=$(cat $common $client $server | wc -l | awk '{print $1}')
echo "Built ($locs lines of code)"
printf "=%.0s" $(seq 10)
echo ""