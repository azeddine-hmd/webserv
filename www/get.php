<html>
<body>

<form method="get" action="<?php echo $_SERVER['PHP_SELF'];?>">
  Name: <input type="text" name="fname">
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
?>
</body>
</html>