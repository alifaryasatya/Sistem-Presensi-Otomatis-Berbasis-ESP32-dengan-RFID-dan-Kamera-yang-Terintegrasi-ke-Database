function doPost(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Sheet1"); // Ganti dengan nama sheet kamu
  var uid = e.parameter.uid; // Mengambil parameter uid
  var name = e.parameter.name; // Mengambil parameter name


  // Periksa apakah UID dan nama ada
  if (uid && name) {
    // Menambahkan data UID, nama, dan tanggal/waktu ke sheet
    sheet.appendRow([uid, name, new Date()]);
    return ContentService.createTextOutput("Data diterima: " + uid + ", " + name);
  } else {
    return ContentService.createTextOutput("Data tidak lengkap!");
  }
}
