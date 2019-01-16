from typing import *
from types import ModuleType

def exit(retval: Any = ...) -> None:
    raise SystemExit()

def print_exception(exc: BaseException) -> None: ...

path = ...  # type: List[str]
argv = ...  # type: List[str]
version = ...  # type: str
version_info = ...  # type: Tuple[int, int, int]
implementation = ...  # type: Tuple[str, Tuple[int, int, int]]
platform = ...  # type: str
byteorder = ...  # type: str
maxsize = ...  # type: int
modules = ...  # type: Dict[str, ModuleType]
