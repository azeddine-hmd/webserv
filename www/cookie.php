<?php
    $cookie_name = "name";
    $cookie_value = "Mourad";
    setcookie($cookie_name, $cookie_value, time() + (86400 * 30), "/");
    setcookie("first_name", "awa.jpeg", time() + (86400 * 30), "/");
    setcookie("last_name", "ayaya", time() + (86400 * 30), "/");


?>

<html>
<body>

    <?php
    if(!isset($_COOKIE[$cookie_name])) {
      echo "Cookie named '" . $cookie_name . "' is not set!";
    } else {
      echo "Cookie '" . $cookie_name . "' is set!<br>";
      echo "Value is: " . $_COOKIE[$cookie_name];
    }
    ?>

</body>
</html>