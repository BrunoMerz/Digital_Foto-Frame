<?php
session_start();
$sessionid = session_id();

if (!isset($_SESSION['checksum'])) {
    $_SESSION['checksum'] = 0;
}

$session_checksum = $_SESSION['checksum'];

// Nur Requests von dieser IP akzeptieren
$allowed_ips = ['37.49.39.206']; // öffentliche IP deines Heimnetzwerks
if (!in_array($_SERVER['REMOTE_ADDR'], $allowed_ips)) {
    http_response_code(403);
    exit("Forbidden: IP not allowed");
}

// Token prüfen
$valid_tokens = ["mss1", "mss2"];
$token = $_GET['token'] ?? null;

if (!in_array($token, $valid_tokens, true)) {
    http_response_code(403);
    exit("Forbidden: Invalid token");
}

// -------------------
// 📁 Bilder sammeln
// -------------------

$dir = $_SERVER['DOCUMENT_ROOT'] . '/wp-content/uploads/Dpf/';
$all_files = glob($dir . '*');
$files = preg_grep('/\.(jpg|jpeg|png)$/i',$all_files);
sort($files);
$checksum = md5(implode('|', $files));
$_SESSION['checksum'] = $checksum;

if($checksum == $session_checksum) {
	$state = "no_news";
} else {
	$state = "new_files";
}

if($token == "mss2") {
	header('Content-Type: application/json');
	echo json_encode([
		"state" => $state,
		"checksum" => $checksum,
		"session_checksum" => $session_checksum,
		"sessionid" => $sessionid,
	]);
	exit;
}
	
$imageList = [];


foreach ($files as $file) {
	$imageList[] =  basename($file);
}


// -------------------
// 📦 JSON Antwort
// -------------------


header('Content-Type: application/json');
echo json_encode([
	"checksum" => $checksum,
	"session_checksum" => $session_checksum,
	"sessionid" => $sessionid,
	"path" => "https://" . $_SERVER['HTTP_HOST'] . "/wp-content/uploads/Dpf/",
    "count" => count($imageList),
    "images" => $imageList
]);
?>