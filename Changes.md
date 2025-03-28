
# Changes - CSE_FT6206


#
### **+05:30 07:34:53 PM 28-03-2025, Friday**

  - Removed all redundant touch point data variables.
  - Added `fastReadData()` that will only read the specified touch point.

#
### **+05:30 08:48:14 AM 28-03-2025, Friday**

  - Updated the examples.
    - Both touch points are now printed.
    - Also prints the touch state.
  - Added `Test.ino` sketch for internal testing.
  - Data is not read if ID is invalid in `isTouched()`.
    - The function returns immediately.
    - The touch point state is now correctly checked.
  - Data is not read if ID is invalid in `getPoint()`.
    - The function returns immediately.
  - New version 🆕 `v0.0.3`.

#
### **+05:30 12:16:22 AM 27-03-2025, Thursday**

  - Updated the library to support `CSE_Touch` and `CSE_UI` libraries.
  - The library now behaves like the `CSE_CST328` touch library.
  - Added Arduino library specification files.
  - Added license file.
  - Updated the examples.
    - Examples are tested with `FireBeetle-ESP32E` board and found to be working.
  - Updated Readme file.
  - New version 🆕 `v0.0.2`.
