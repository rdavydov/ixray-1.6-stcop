#ifndef screenspacecontactshadows_hlsl_included
#define screenspacecontactshadows_hlsl_included

float3 gbuf_unpack_position(float2 uv, float depth)
{
	uv = uv * 2.f - 1.f; depth = abs(depth);
	return float3(uv * pos_decompression_params.xy, 1.f) * depth;
}

float2 gbuf_unpack_uv(float3 position)
{
	position.xy /= pos_decompression_params.xy * position.z;
	return saturate(position.xy * 0.5 + 0.5);
}

float GetBorderAtten(float2 tc, float att = 0.025f)
{
	float2 borderDist = 1.0f - abs(tc * 2.0f - 1.0f);
	float2 borderAtten = saturate(borderDist / att);
	return borderAtten.x * borderAtten.y;
}

float hash(float  n) { return frac(sin(n) * 43758.5453123f); }
float hash(float2 n) { return frac(sin(dot(n, float2(15.41, 64.32))) * 43758.5453123f); }
float hash(float3 n) { return frac(sin(dot(n, float3(15.41, 64.32, 124.45))) * 43758.5453123f); }

void RayTraceContactShadow(float2 tc, float3 P, float3 N, float3 L2P, inout float shadow) {
	float shadow_fade = smoothstep(4.0, 3.6, dot(P, P));
	if(shadow_fade > 0.0 && shadow > 0.0)
	{
		float smap = 1.0;
		P.xyz *= 0.99;
		int steps = 40;
		float3 unit_vec = 0.07 * normalize(L2P) / steps;
		int i = 0; float2 uv = 0.0;
		while(++i < steps) {
			float jstep = hash(P * timers.x) * 0.5 + 0.75;
			P -= unit_vec * jstep;
			uv = gbuf_unpack_uv(P);
			if(all(min(uv, 1.0 - uv))) {
				float HitDepth = tex2Dlod(s_position, float4(uv, 0, 0)).z;
				if(HitDepth < P.z && HitDepth > 0.01) {
					smap -= 0.2;
					if(smap <= 0.0) {
						i = steps;
						smap = 0.0f;
					}
				}
			} else {
				i = steps;
			}
		} 
		shadow_fade *= GetBorderAtten(uv, 0.025);
		shadow = lerp(shadow, smap * shadow, shadow_fade);
	}
}

#endif