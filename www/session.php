<?php
session_start();
?>
<!DOCTYPE html>
<html>
<body>

<?php
if(isset($_SESSION["visits"])) {
    $_SESSION["visits"] = $_SESSION["visits"] + 1;
} else {
    $_SESSION["visits"] = 1;
}

// print_r($_SESSION);
//print session visites
echo "<h1>You have visited this page " . $_SESSION["visits"] . " times.</h1>";
?>


</body>
</html>
