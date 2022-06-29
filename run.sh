if [[ "$(id -u)" -ne 0 ]]; then
  >&2 echo "NOTE: Most eBPF programs cannot be loaded into the kernel by un-privileged
users. Please re-run this script as root.

Exiting."
  exit 1
fi

DEMO_SERVER_URL=http://localhost:4040/myprocess/

PROJECT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
declare -a PIDS
CORE_COUNT=$(awk '/^core id/{print $4}' /proc/cpuinfo | sort -u | wc -l)

killpids(){
  echo "Stopping child processes."
  set +e
  for pid in "${PIDS[@]}"; do
    if ps "$pid" >/dev/null; then
      kill -TERM "$pid"
    fi
  done
}

cleanup(){
  echo "Cleaning up."
  set +e
  for pid in "${PIDS[@]}"; do
    if ps "$pid" >/dev/null; then
      kill -TERM "$pid"
    fi
  done
  rm -rf "$BUILD_DIR"
}

trap killpids INT
trap killpids HUP
trap cleanup EXIT

# Create a temporary directory relative to root's home directory.
BUILD_DIR="$(mktemp -d -p ~)"
DEMO_SERVER_DIR="$BUILD_DIR/demo_server"

echo "*** Creating Python virtual environment for demo API server ***"
cp -a "$PROJECT_DIR/tools/demo_server" "$BUILD_DIR"
python3 -m venv "$DEMO_SERVER_DIR/venv"
source "$DEMO_SERVER_DIR/venv/bin/activate"

# Install python dependencies for demo_server in the newly created virtualenv
pip3 install -r "$DEMO_SERVER_DIR/requirements.txt"

# Configure and build ebpf demo application
cd "$BUILD_DIR"
echo "
*** Configuring levo-ebpf-demo build ***"
cmake -G "Unix Makefiles" "$PROJECT_DIR"
echo "*** Building levo-ebpf-demo ***"
make -j "$CORE_COUNT"

echo "*** Starting demo API server ***"
cd "$DEMO_SERVER_DIR"
source venv/bin/activate
python3 -m openapi_server &
PIDS[0]=$!

while ps ${PIDS[0]} >/dev/null 2>&1 && ! curl --head $DEMO_SERVER_URL >/dev/null 2>&1; do
  sleep 1
done

echo "
*** Starting levo-ebpf-demo application ***"
cd "$BUILD_DIR"
./src/levo-ebpf-demo -p ${PIDS[0]} -b src/bpf/bpf_progs.bpf.o &
PIDS[1]=$!

sleep 2

echo "

*** Demo API server and levo-ebf-demo started. ***

Open another terminal window and run...

curl $DEMO_SERVER_URL

...to exercise levo-ebpf-demo
Captured socket data will appear on the standard output.

"
wait "${PIDS[@]}"
