// Generated from vertex_program/modules/fog.inc by asm2hlsl. Included inside main().

// ======================================================================
// inputs:
//
//   r11.w    = distance squared from vertex to camera
//
// outputs:
//
//   oFog
//
// ======================================================================
// ----------------------------------------------------------------------
// calculate fog
// (distance)^2 * (density)^2 = (distance * density)^2
	r0.w = (r11.w * cFog.w).x;
// log2(e) * (distance * density)^2
	r0.w = (cLog2e * r0.w).x;
// 2^(log2(e) * (distance * density)^2) = (2^log2(e))^((distance * density)^2) = e^((distance * density)^2)
	r0.w = ((exp2(r0.w)).xxxx).x;
// 1 / (e^((distance * density)^2))
	vsOutput.fog = ((1.0f / (r0.w))).xxxx;
