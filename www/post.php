
<html>
<body>

<form method="post" action="<?php echo $_SERVER['PHP_SELF'];?>">
  Name: <input type="text" name="fname"><br>
  <!-- Name: <input type="text" name="fname2"><br>
  Name: <input type="text" name="fname3"><br> -->
  <input type="submit">
</form>

<form action="upload.php" method="post" enctype="multipart/form-data">
        Select image to upload:
        <input type="file" name="fileToUpload" id="fileToUpload">
        <input type="submit" value="Upload Image" name="submit">
</form>

<?php
// if ($_SERVER["REQUEST_METHOD"] == "POST") {
  // collect value of input field
//   if (isset($_POST['fname']) && isset($_POST['fname2']) && isset($_POST['fname3']))
if (isset($_POST['fname']))
  {
    $name = $_POST['fname'];
    // $name2 = $_POST['fname2'];
    // $name3 = $_POST['fname3'];
  }
  
  if (empty($name)) {
    echo "Name is empty";
  } else {
    echo $name;
  }
//   echo "<br>";
//   if (empty($name2)) {
//     echo "Name2 is empty";
//   } else {
//     echo $name2;
//   }
//   echo "<br>";
//   if (empty($name3)) {
//     echo "Name3 is empty";
//   } else {
//     echo $name3;
//   }
//   echo "<br>";
// }
?>

</body>
</html>