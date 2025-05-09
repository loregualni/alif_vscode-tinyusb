#!/usr/bin/env python

from typing import Optional, Union, Tuple, Dict, TypeAlias

import fire
import usb.core
from usb.core import Device, Configuration, Interface, Endpoint
from usb.util import (
    build_request_type,
    endpoint_direction,
    endpoint_type,
    CTRL_IN,
    CTRL_OUT,
    CTRL_TYPE_VENDOR,
    CTRL_RECIPIENT_DEVICE,
    ENDPOINT_IN,
    ENDPOINT_OUT,
    ENDPOINT_TYPE_BULK,
)

InterfaceIndex: TypeAlias = Tuple[int, int]
EndpointMap: TypeAlias = Dict[str, Endpoint]
EndpointDir: TypeAlias = int
EndpointXfer: TypeAlias = int

DEFAULT_VENDOR_ID = 0xCAFE
DEFAULT_PRODUCT_ID = 0x4010
DEFAULT_INTERFACE_INDEX = (0, 0)
DEFAULT_TIMEOUT = 1000

VENDOR_REQUEST_IN = build_request_type(
    CTRL_IN,
    CTRL_TYPE_VENDOR,
    CTRL_RECIPIENT_DEVICE,
)
VENDOR_REQUEST_READ = 0x01

VENDOR_REQUEST_OUT = build_request_type(
    CTRL_OUT,
    CTRL_TYPE_VENDOR,
    CTRL_RECIPIENT_DEVICE,
)
VENDOR_REQUEST_WRITE = 0x02


def _u32_from_str(val: str) -> int:
    _val = val
    base = 10
    if _val.startswith("0x"):
        base = 16
        _val = _val.replace("0x", "")
    res = int(_val, base=base)
    if res >= 0xFFFFFFFF:
        raise ValueError(
            f"U32 from string conversion failed, {val} is too big for a 32-bit number."
        )
    return res


def _get_usb_device(vid: int, pid: int) -> Device:
    dev = usb.core.find(idVendor=vid, idProduct=pid)
    if dev is None:
        raise ValueError(
            f"No device with vid: '{vid}' and pid: '{pid}' has been found."
        )
    return dev


def _get_usb_configuration(dev: Device) -> Configuration:
    dev.set_configuration()
    return dev.get_active_configuration()


def _get_usb_interface(
    cfg: Configuration, index: Optional[InterfaceIndex] = None
) -> Interface:
    if index is None:
        index = DEFAULT_INTERFACE_INDEX
    return cfg[index]


def _get_usb_endpoint(itf: Interface, dir: EndpointDir, xfer: EndpointXfer) -> Endpoint:
    for ep in itf:
        if (endpoint_direction(ep.bEndpointAddress) == dir) and (
            endpoint_type(ep.bmAttributes) == xfer
        ):
            return ep
    raise ValueError(
        f"No endpoint with dir: '{hex(dir)}' and xfer: '{hex(xfer)}' has been found."
    )


def _read_usb_endpoint_bytes(
    dev: Device, ep_in: Endpoint, timeout: int = DEFAULT_TIMEOUT
) -> bytes:
    return dev.read(ep_in.bEndpointAddress, ep_in.wMaxPacketSize, timeout=timeout)


def _write_usb_endpoint_bytes(
    dev: Device, ep_in: Endpoint, data: bytes, timeout: int = DEFAULT_TIMEOUT
) -> int:
    return dev.write(ep_in.bEndpointAddress, data, timeout=timeout)


def _usb_ctrl_transfer_out(
    dev: Device, out_data: bytes, timeout: int = DEFAULT_TIMEOUT
) -> int:
    return dev.ctrl_transfer(
        bmRequestType=VENDOR_REQUEST_OUT,
        bRequest=VENDOR_REQUEST_WRITE,
        data_or_wLength=out_data,
        timeout=timeout,
    )


def _usb_ctrl_transfer_in(
    dev: Device, in_data_len: int, timeout: int = DEFAULT_TIMEOUT
) -> bytes:
    return dev.ctrl_transfer(
        bmRequestType=VENDOR_REQUEST_IN,
        bRequest=VENDOR_REQUEST_READ,
        data_or_wLength=in_data_len,
        timeout=timeout,
    )


def main(
    vid: Optional[Union[int, str]] = None,
    pid: Optional[Union[int, str]] = None,
) -> None:
    """
    ShowCar device USB testing CLI tool.

    Args:
        vid (int | str | None): 32-bit vendor ID number, either in HEX or DEC
            value. If None, '0xcafe' is used. Defaults to None.
        pid (int | str | None): 32-bit product ID number, either in HEX or DEC
            value. If None, '0xcafe' is used. Defaults to None.
    """
    if vid is None:
        vid = DEFAULT_VENDOR_ID
    if isinstance(vid, str):
        vid = _u32_from_str(vid)

    if pid is None:
        pid = DEFAULT_PRODUCT_ID
    if isinstance(pid, str):
        pid = _u32_from_str(pid)

    dev = _get_usb_device(vid, pid)
    cfg = _get_usb_configuration(dev)
    itf = _get_usb_interface(cfg)
    bulk_in = _get_usb_endpoint(itf, ENDPOINT_IN, ENDPOINT_TYPE_BULK)
    bulk_out = _get_usb_endpoint(itf, ENDPOINT_OUT, ENDPOINT_TYPE_BULK)

    while True:
        try:
            # TODO: find out why this does not work
            send = b"HEY"
            _usb_ctrl_transfer_out(dev, send)
            print(f"CTRL - Sent {len(send):>2} bytes -> '{bytes(send).decode()}'")

            recv = _usb_ctrl_transfer_in(dev, in_data_len=64)
            print(f"CTRL - Recv {len(recv):>2} bytes -> '{bytes(recv).decode()}'")

            send = b"HELLO"
            _write_usb_endpoint_bytes(dev, bulk_out, send)
            print(f"BULK - Sent {len(send):>2} bytes -> '{bytes(send).decode()}'")

            recv = _read_usb_endpoint_bytes(dev, bulk_in)
            print(f"BULK - Recv {len(recv):>2} bytes -> '{bytes(recv).decode()}'")

        except (usb.core.USBError, usb.core.USBTimeoutError) as e:
            print(str(e))


if __name__ == "__main__":
    fire.Fire(main)
