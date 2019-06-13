<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Welcome extends CI_Controller {

	public function index()
	{
		$this->load->helper('url');

		$this->load->database();

		$query = $this->db->get('hashes');

		$data['database'] = $query->result();		

		$this->load->view('welcome_message', $data);
	}

	public function display($param=NULL)
	{
		if (is_null($param)) {
			die("Paramater wasn't passed properly");
		}

		$this->load->helper('url');

		$this->load->database();

		$query = $this->db->get_where('hashes', array('hashID' => $param));

		$data['database'] = $query->result();		

		$this->load->view('welcome_message', $data);
	}
}
