// From https://www.shadertoy.com/view/XssGD7

void main()
{
	//vec2 uv = fragCoord.xy / iResolution.xy;
	
	// Sobel operator
	float offset = onePixel;
	vec3 o = vec3(-offset, 0.0, offset);
	vec4 gx = vec4(0.0);
	vec4 gy = vec4(0.0);
	vec4 t;
	gx += texture2D(iFrame, uv + o.xz);
	gy += gx;
	gx += 2.0*texture2D(iFrame, uv + o.xy);
	t = texture2D(iFrame, uv + o.xx);
	gx += t;
	gy -= t;
	gy += 2.0*texture2D(iFrame, uv + o.yz);
	gy -= 2.0*texture2D(iFrame, uv + o.yx);
	t = texture2D(iFrame, uv + o.zz);
	gx -= t;
	gy += t;
	gx -= 2.0*texture2D(iFrame, uv + o.zy);
	t = texture2D(iFrame, uv + o.zx);
	gx -= t;
	gy -= t;
	vec4 grad = sqrt(gx * gx + gy * gy);

    vec4 original = texture2D(iFrame, uv);
    grad.a *= smoothstep(0., 0.5, iIntensity);
    original.a *= 1. - smoothstep(0.5, 1., iIntensity);

    fragColor = composite(original, grad);
}
