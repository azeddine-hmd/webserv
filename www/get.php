<html>
<body>

<form method="get" action="<?php echo $_SERVER['PHP_SELF'];?>">
  Name: <input type="text" name="fname">
  <input type="submit">
  Name: <input type="text" name="fname2">
  <input type="submit">
</form>

<?php
    if (isset($_GET['fname'])) {
        $name = $_GET['fname'];

        if (empty($name))
            echo "Name is empty";
        else
            echo $name;
    }
    echo "\n";
    if (isset($_GET['fname2'])) {
        $name2 = $_GET['fname2'];

        if (empty($name2))
            echo "Name is empty";
        else
            echo $name2;
    }

	for ($i = 0; $i < 1000; $i++) {
		echo "<h3>$i</h3>\n";
	}
?>
</body>
</html>
