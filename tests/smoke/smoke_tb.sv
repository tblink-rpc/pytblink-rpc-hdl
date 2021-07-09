/****************************************************************************
 * smoke_tb.sv
 ****************************************************************************/
`ifdef NEED_TIMESCALE
`timescale 1ns/1ns
`endif /* NEED_TIMESCALE */

/**
 * Module: smoke_tb
 * 
 * TODO: Add module documentation
 */
module smoke_tb(input clock);

`ifdef HAVE_HDL_CLOCKGEN
	reg clock_r = 0;
`ifdef NEED_TIMESCALE
	always clock_r = #10 ~clock_r;
`else
	always clock_r = #10ns ~clock_r;
`endif
	assign clock = clock_r;
`endif
	
`ifdef IVERILOG
	`include "iverilog_control.svh"
`endif


endmodule


