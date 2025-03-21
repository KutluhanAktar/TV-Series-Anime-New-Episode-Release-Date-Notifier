<?php

$conn_database = mysqli_connect("localhost", "root", "bot", "tvseries");

function checkDatabase($table, $conn){
		$sql = "SHOW TABLES FROM `tvseries` LIKE '$table'";
	    $check = mysqli_num_rows(mysqli_query($conn, $sql));
		return ($check > 0) ? true : false;
}

function createTable($table_name, $conn){
	if(!checkDatabase($table_name, $conn)){
		$sql = "CREATE TABLE `$table_name`(
		id int(11) AUTO_INCREMENT PRIMARY KEY NOT NULL,
		series_name varchar(255) NOT NULL,
		tv_id varchar(255) NOT NULL,
		status varchar(255) NOT NULL
	    );";
			
		if(mysqli_query($conn, $sql)){
			echo "Database Table Created!";
		}else{
			echo "Error!";
		}
	}else{
		echo "Database Table Found!";
	}
}

createTable("series", $conn_database);

?>
