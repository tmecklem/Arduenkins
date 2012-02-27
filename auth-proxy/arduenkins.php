<?php
    $urlStart = "http://localhost:8081/jenkins/job/";
    $urlEnd = "/api/json?tree=color";
    $username = "arduenkins";
    $apiToken = "get-this-from-jenkins";

    $job = $_GET["job"];
    $url = $urlStart . $job .$urlEnd;
    $process = curl_init($url);
    curl_setopt($process, CURLOPT_USERPWD, $username . ":" . $apiToken);
    curl_setopt($process, CURLOPT_RETURNTRANSFER, TRUE);
    $response = curl_exec($process);
    curl_close($process);

    echo $response;
?>
