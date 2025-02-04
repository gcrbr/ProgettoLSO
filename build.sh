echo "Building..."
gcc client/*.c common/*.c -o run_client
gcc server/*.c common/*.c -o run_server
echo "Built"