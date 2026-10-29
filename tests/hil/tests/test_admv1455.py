"""ADMV1455 RF test: step a DSA and check the output power drops.

Flow: build + load firmware (SD-Mux boot), apply a tone from the signal generator
to the RF input, read the IF output peak on the spectrum analyzer with the DSA at
0 dB, step the DSA down, and assert the peak falls by the same amount.

    pytest tests/hil/tests/test_admv1455.py \
        --lg-env tests/hil/env/zcu102.yaml --noos-loader sdmux \
        --noos-project eval-admv1455 --noos-build iio --noos-board zcu102 \
        --noos-xsa /path/system_top.xsa

The LO is assumed already running at a fixed setting and the supply on. The IIO
serial backend needs the console tty, so the console is released after boot.
"""

import time

import pytest

pytestmark = [
    pytest.mark.iio_hardware(["admv1455"]),
    pytest.mark.iio_carrier(["zcu102"]),
]

DSA_ATTR = "direct_dsa1_gain"   # attribute on the "rf" channel (altvoltage0)
DSA_STEP_DB = 6
TOLERANCE_DB = 1.5
SETTLE_S = 1.0
MIN_SNR_DB = 10                 # tone must stand out of the noise floor


def test_build_admv1455(firmware):
    """Firmware builds and produces a boot artifact; no hardware is touched."""
    assert firmware.elf is not None or firmware.boot_bin is not None


@pytest.fixture
def iio_ctx(request, target, loaded_firmware):
    """libiio context to the booted IIO firmware (console released first)."""
    iio = pytest.importorskip("iio", reason="libiio python bindings unavailable")
    uri = request.config.getoption("--noos-iio-uri")
    if not uri:
        port = target.get_resource("RawSerialPort")
        uri = f"serial:{port.port},{port.speed}"
    # libiio's serial backend needs the tty the console driver holds.
    target.deactivate(loaded_firmware["console"])

    deadline = time.monotonic() + 60
    while True:
        try:
            return iio.Context(uri)
        except Exception:
            if time.monotonic() > deadline:
                raise
            time.sleep(2)


@pytest.fixture
def rf_bench(request, target):
    """Signal generator driving the input and analyzer set up on the output."""
    cfg = request.config.getoption
    siggen = target.get_driver("PyInstSignalGeneratorDriver")
    fsu = target.get_driver("PyInstSpectrumAnalyzerDriver")
    siggen.output_enabled = False
    siggen.frequency = cfg("--rf-in-freq")
    siggen.power = cfg("--rf-in-power")
    fsu.center_frequency = cfg("--rf-out-freq")
    fsu.span = cfg("--rf-span")
    fsu.reference_level = cfg("--rf-ref-level")
    siggen.output_enabled = True
    yield fsu
    siggen.output_enabled = False


def _peak_dbm(fsu):
    _freqs, levels = fsu.get_trace()
    return float(levels.max()), float(sorted(levels)[len(levels) // 2])


def test_dsa_attenuates_output(iio_ctx, rf_bench):
    """Stepping the DSA down by DSA_STEP_DB lowers the output peak by about that much."""
    chan = iio_ctx.find_device("admv1455").find_channel("altvoltage0", False)
    attr = chan.attrs[DSA_ATTR]
    original = attr.value
    try:
        attr.value = "0dB"
        time.sleep(SETTLE_S)
        ref_peak, noise = _peak_dbm(rf_bench)
        assert ref_peak - noise >= MIN_SNR_DB, (
            f"no tone at the analyzer: peak {ref_peak:.1f} dBm vs median {noise:.1f} dBm")

        attr.value = f"-{DSA_STEP_DB}dB"
        time.sleep(SETTLE_S)
        att_peak, _ = _peak_dbm(rf_bench)
    finally:
        attr.value = original

    drop = ref_peak - att_peak
    assert abs(drop - DSA_STEP_DB) <= TOLERANCE_DB, (
        f"{DSA_ATTR} 0dB -> -{DSA_STEP_DB}dB: output dropped {drop:.2f} dB "
        f"({ref_peak:.2f} -> {att_peak:.2f} dBm), expected {DSA_STEP_DB} ±{TOLERANCE_DB} dB")
