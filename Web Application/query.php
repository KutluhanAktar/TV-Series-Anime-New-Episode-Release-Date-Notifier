<?php

error_reporting(0);

class query{
	
	const ROOT = "https://api.tvmaze.com/shows/";
	const PATH = "/episodesbydate?date=";
	
	public $conn, $table_name;
	
	public function define_user($conn, $table_name){
		$this->conn = $conn;
		$this->table_name = $table_name;
	}
	
	private function check_ep_release_date($API_ID, $series_name, $status){
		$date = date("Y-m-d");
		$URL = self::ROOT.$API_ID.self::PATH.$date;
		
		if($get_content = json_decode(file_get_contents((String)$URL))){
			if($status != "Released"){
				echo "%".$series_name."%".$get_content[0]->season."%".$get_content[0]->number."%".$get_content[0]->name."%";
				$sql = "UPDATE `$this->table_name` SET `status`='Released' WHERE series_name='$series_name'";
				mysqli_query($this->conn, $sql);
			}
		}else{
			if($status != "Pending"){
				$sql = "UPDATE `$this->table_name` SET `status`='Pending' WHERE series_name='$series_name'";
				mysqli_query($this->conn, $sql);
			}
		}
	}
	
	public function track_db_entries(){
		$sql = "SELECT * FROM `$this->table_name` ORDER BY `id` DESC";
		if($result = mysqli_query($this->conn, $sql)){
			$check = mysqli_num_rows($result);
			if($check > 0){
				while($row = mysqli_fetch_assoc($result)){
					$this->check_ep_release_date($row['tv_id'], $row['series_name'], $row['status']);
				}
			}else{
				echo "No Entry Found!";
			}
		}else{
			echo "No Database Found!";
		}
	}
}

// Define the connection settings and the table name.
$conn_database = mysqli_connect("localhost", "root", "bot", "tvseries");
$table = "series";

// Define the object to check release date for each registered series / anime.
$q = new query();
$q->define_user($conn_database, $table);
$q->track_db_entries();

?>