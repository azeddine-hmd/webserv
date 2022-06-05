<html>
<body>

<form method="get" action="/cgi/GET.php">
  Name: <input type="text" name="testname">
  <input type="submit">
</form>

<?php
if ($_SERVER["REQUEST_METHOD"] == "GET") {
  // collect value of input field
  $name = $_GET['testname'];
  
  if (empty($name)) {
    echo "Name is empty";
  } else {
    echo $name;
  }
}
?>

</body>
</html>