"""Xilinx SD-Mux loader: write BOOT.BIN to the SD card, boot the board from SD.

Powers the board off, routes the SD card to the host with the USB SD-Mux, copies
BOOT.BIN onto the boot partition, routes the card back to the DUT and powers the
board on. Uses the PowerProtocol, USBSDMuxDriver and MassStorageDriver from the
labgrid env YAML; if any is missing the test skips.
"""

from __future__ import annotations

import os
import time

_PARTITION_TIMEOUT = 30


def _wait_for_path(path, timeout):
    deadline = time.monotonic() + timeout
    while not os.path.exists(path):
        if time.monotonic() > deadline:
            raise TimeoutError(f"{path} did not appear within {timeout}s")
        time.sleep(0.5)


class SDMuxLoader:
    name = "sdmux"

    def load(self, artifacts, target, options) -> None:
        import pytest
        # require() before touching the target so a missing artifact is a clear error.
        boot_bin = artifacts.require("boot_bin")

        try:
            power = target.get_driver("PowerProtocol")
            mux = target.get_driver("USBSDMuxDriver")
            storage = target.get_driver("MassStorageDriver")
        except Exception as exc:
            pytest.skip(f"sdmux loader needs PowerProtocol, USBSDMuxDriver and "
                        f"MassStorageDriver in the labgrid env ({exc})")

        power.off()
        mux.set_mode("host")
        try:
            _wait_for_path(storage.partition, _PARTITION_TIMEOUT)
            storage.mount_partition()
            try:
                storage.copy_file(str(boot_bin), "BOOT.BIN")
            finally:
                os.sync()
                storage.unmount_partition()
        finally:
            mux.set_mode("dut")
        power.on()
