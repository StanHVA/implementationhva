<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Welcome extends CI_Controller {

	public function __construct(){
        parent::__construct();
		$this->load->helper('url');
		$this->load->database();
	}

	public function index()
	{

		$query = $this->db->get('hashes');

		$data['database'] = $query->result();		

		$this->load->view('welcome_message', $data);
	}

	public function display($param=NULL)
	{
		$param = $this->input->post('param');
		if (is_null($param)) {
			die("Paramater wasn't passed properly");
		}

		$query = $this->db->get_where('hashes', array('hashID' => $param));

		$data['hashid'] = $param;
		$data['database'] = $query->result();		

		$this->load->view('welcome_message', $data);
	}
}
