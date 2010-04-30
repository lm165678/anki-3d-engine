#include <string.h>
#include "Material.h"
#include "Resource.h"
#include "Scanner.h"
#include "Parser.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "Renderer.h"


/// Customized @ref ERROR used in @ref Material class
#define MTL_ERROR( x ) ERROR( "Material (" << getRsrcPath() << getRsrcName() << "): " << x );


//=====================================================================================================================================
// Blending stuff                                                                                                                     =
//=====================================================================================================================================
struct BlendParam
{
	int glEnum;
	const char* str;
};

static BlendParam blendingParams [] =
{
	{GL_ZERO, "GL_ZERO"},
	{GL_ONE, "GL_ONE"},
	{GL_DST_COLOR, "GL_DST_COLOR"},
	{GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR"},
	{GL_SRC_ALPHA, "GL_SRC_ALPHA"},
	{GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA"},
	{GL_DST_ALPHA, "GL_DST_ALPHA"},
	{GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA"},
	{GL_SRC_ALPHA_SATURATE, "GL_SRC_ALPHA_SATURATE"},
	{GL_SRC_COLOR, "GL_SRC_COLOR"},
	{GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR"}
};

const int BLEND_PARAMS_NUM = 11;

static bool searchBlendEnum( const char* str, int& gl_enum )
{
	for( int i=0; i<BLEND_PARAMS_NUM; i++ )
	{
		if( !strcmp( blendingParams[i].str, str) )
		{
			gl_enum = blendingParams[i].glEnum;
			return true;
		}
	}
	return false;
}


//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool Material::load( const char* filename )
{
	Scanner scanner;
	if( !scanner.loadFile( filename ) ) return false;

	const Scanner::Token* token;

	do
	{
		token = &scanner.getNextToken();

		//** SHADER_PROG **
		if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "SHADER_PROG" ) )
		{
			if( shaderProg ) ERROR( "Shader program already loaded" );

			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			shaderProg = Rsrc::shaders.load( token->getValue().getString() );
		}
		//** DEPTH_MATERIAL **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "DEPTH_PASS_MATERIAL" ) )
		{
			if( dpMtl ) ERROR( "Depth material already loaded" );

			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			dpMtl = Rsrc::materials.load( token->getValue().getString() );
		}
		//** BLENDS **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "BLENDS" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			blends = token->getValue().getInt();
		}
		//** REFRACTS **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "REFRACTS" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			refracts = token->getValue().getInt();
		}
		//** BLENDING_SFACTOR **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "BLENDING_SFACTOR" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_IDENTIFIER )
			{
				PARSE_ERR_EXPECTED( "identifier" );
				return false;
			}
			int gl_enum;
			if( !searchBlendEnum(token->getValue().getString(), gl_enum) )
			{
				PARSE_ERR( "Incorrect blending factor \"" << token->getValue().getString() << "\"" );
				return false;
			}
			blendingSfactor = gl_enum;
		}
		//** BLENDING_DFACTOR **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "BLENDING_DFACTOR" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_IDENTIFIER )
			{
				PARSE_ERR_EXPECTED( "identifier" );
				return false;
			}
			int gl_enum;
			if( !searchBlendEnum(token->getValue().getString(), gl_enum) )
			{
				PARSE_ERR( "Incorrect blending factor \"" << token->getValue().getString() << "\"" );
				return false;
			}
			blendingDfactor = gl_enum;
		}
		//** DEPTH_TESTING **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "DEPTH_TESTING" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			depthTesting = token->getValue().getInt();
		}
		//** WIREFRAME **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "WIREFRAME" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			wireframe = token->getValue().getInt();
		}
		//** CASTS_SHADOW **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "CASTS_SHADOW" ) )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			castsShadow = token->getValue().getInt();
		}
		//** USER_DEFINED_VARS **
		else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "USER_DEFINED_VARS" ) )
		{
			// first check if the shader is defined
			if( shaderProg == NULL )
			{
				PARSE_ERR( "You have to define the shader program before the user defined vars" );
				return false;
			}

			// read {
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}
			// loop all the vars
			do
			{
				// read the name
				token = &scanner.getNextToken();
				if( token->getCode() == Scanner::TC_RBRACKET ) break;

				if( token->getCode() != Scanner::TC_IDENTIFIER )
				{
					PARSE_ERR_EXPECTED( "identifier" );
					return false;
				}

				string varName;
				varName = token->getValue().getString();

				userDefinedVars.push_back( UserDefinedVar() ); // create new var
				UserDefinedVar& var = userDefinedVars.back();

				// check if the uniform exists
				if( !shaderProg->uniVarExists( varName.c_str() ) )
				{
					PARSE_ERR( "The variable \"" << varName << "\" is not an active uniform" );
					return false;
				}

				var.sProgVar = shaderProg->getUniVar( varName.c_str() );

				// read the values
				switch( var.sProgVar->getGlDataType() )
				{
					// texture
					case GL_SAMPLER_2D:
						token = &scanner.getNextToken();
						if( token->getCode() == Scanner::TC_STRING )
						{
							var.value.texture = Rsrc::textures.load( token->getValue().getString() );
						}
						else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "IS_FAI" ) )
						{
							var.value.texture = &R::Is::fai;
						}
						else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "MS_NORMAL_FAI" ) )
						{
							var.value.texture = &R::Ms::normalFai;
						}
						else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "MS_DEPTH_FAI" ) )
						{
							var.value.texture = &R::Ms::depthFai;
						}
						else if( token->getCode() == Scanner::TC_IDENTIFIER && !strcmp( token->getValue().getString(), "PPS_FAI" ) )
						{
							var.value.texture = &R::Pps::fai;
						}
						else
						{
							PARSE_ERR_EXPECTED( "string or IS_FAI or MS_NORMAL_FAI or MS_DEPTH_FAI or PPS_FAI" );
							return false;
						}
						break;
					// float
					case GL_FLOAT:
						token = &scanner.getNextToken();
						if( token->getCode() == Scanner::TC_NUMBER && token->getDataType() == Scanner::DT_FLOAT )
							var.value.float_ = token->getValue().getFloat();
						else
						{
							PARSE_ERR_EXPECTED( "float" );
							return false;
						}
						break;
					// vec2
					case GL_FLOAT_VEC2:
						ERROR( "Unimplemented" );
						break;
					// vec3
					case GL_FLOAT_VEC3:
						if( !Parser::parseArrOfNumbers<float>( scanner, true, true, 3, &var.value.vec3[0] ) ) return false;
						break;
					// vec4
					case GL_FLOAT_VEC4:
						if( !Parser::parseArrOfNumbers<float>( scanner, true, true, 4, &var.value.vec4[0] ) ) return false;
						break;
				};

			}while(true); // end loop for all the vars

		}
		// end of file
		else if( token->getCode() == Scanner::TC_EOF )
		{
			break;
		}
		// other crap
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}

	}while( true );

	return additionalInit();
}


//=====================================================================================================================================
// additionalInit                                                                                                                     =
//=====================================================================================================================================
bool Material::additionalInit()
{
	// sanity checks
	if( !shaderProg )
	{
		MTL_ERROR( "Without shader is like cake without sugar (missing SHADER_PROG)" );
		return false;
	}

	// init the attribute locations
	attribLocs.tanget = shaderProg->attribVarExists( "tangent" ) ?  shaderProg->getAttribVar( "tangent" )->getLoc() : -1;
	attribLocs.position = shaderProg->attribVarExists( "position" ) ?  shaderProg->getAttribVar( "position" )->getLoc() : -1;
	attribLocs.normal = shaderProg->attribVarExists( "normal" ) ?  shaderProg->getAttribVar( "normal" )->getLoc() : -1;
	attribLocs.texCoords = shaderProg->attribVarExists( "texCoords" ) ?  shaderProg->getAttribVar( "texCoords" )->getLoc() : -1;

	// vertex weights
	if( shaderProg->attribVarExists( "vertWeightBonesNum" ) )
	{
		attribLocs.vertWeightBonesNum = shaderProg->getAttribVar( "vertWeightBonesNum" )->getLoc();
		attribLocs.vertWeightBoneIds = shaderProg->getAttribVar( "vertWeightBoneIds" )->getLoc();
		attribLocs.vertWeightWeights = shaderProg->getAttribVar( "vertWeightWeights" )->getLoc();
		uniLocs.skinningRotations = shaderProg->getUniVar( "skinningRotations" )->getLoc();
		uniLocs.skinningTranslations = shaderProg->getUniVar( "skinningTranslations" )->getLoc();
	}
	else
	{
		attribLocs.vertWeightBonesNum = attribLocs.vertWeightBoneIds = attribLocs.vertWeightWeights = uniLocs.skinningRotations =
		uniLocs.skinningTranslations = -1;
	}

	return true;
}


//=====================================================================================================================================
// unload                                                                                                                             =
//=====================================================================================================================================
void Material::unload()
{
	Rsrc::shaders.unload( shaderProg );

	// loop all user defined vars and unload the textures
	for( uint i=0; i<userDefinedVars.size(); i++ )
	{
		if( userDefinedVars[i].sProgVar->getType() == GL_SAMPLER_2D )
			Rsrc::textures.unload( userDefinedVars[i].value.texture );
	}
}


//=====================================================================================================================================
// setToDefault                                                                                                                       =
//=====================================================================================================================================
void Material::setToDefault()
{
	shaderProg = NULL;
	blends = false;
	blendingSfactor = GL_ONE;
	blendingDfactor = GL_ZERO;
	depthTesting = true;
	wireframe = false;
	castsShadow = true;
	refracts = false;
	dpMtl = NULL;
	/*depth.shaderProg = NULL;
	depth.alpha_testing_map = NULL;*/
}


//=====================================================================================================================================
// setup                                                                                                                              =
//=====================================================================================================================================
void Material::setup()
{
	shaderProg->bind();

	if( blends )
	{
		glEnable( GL_BLEND );
		//glDisable( GL_BLEND );
		glBlendFunc( blendingSfactor, blendingDfactor );
	}
	else
		glDisable( GL_BLEND );


	if( depthTesting )  glEnable( GL_DEPTH_TEST );
	else                glDisable( GL_DEPTH_TEST );

	if( wireframe )  glPolygonMode( GL_FRONT, GL_LINE );
	else             glPolygonMode( GL_FRONT, GL_FILL );


	// now loop all the user defined vars and set them
	uint texture_Unit = 0;
	Vec<UserDefinedVar>::iterator udv;
	for( udv=userDefinedVars.begin(); udv!=userDefinedVars.end(); udv++ )
	{
		switch( udv->sProgVar->getGlDataType() )
		{
			// texture
			case GL_SAMPLER_2D:
				shaderProg->locTexUnit( udv->sProgVar->getLoc(), *udv->value.texture, texture_Unit++ );
				break;
			// float
			case GL_FLOAT:
				glUniform1f( udv->sProgVar->getLoc(), udv->value.float_ );
				break;
			// vec2
			case GL_FLOAT_VEC2:
				glUniform2fv( udv->sProgVar->getLoc(), 1, &udv->value.vec2[0] );
				break;
			// vec3
			case GL_FLOAT_VEC3:
				glUniform3fv( udv->sProgVar->getLoc(), 1, &udv->value.vec3[0] );
				break;
			// vec4
			case GL_FLOAT_VEC4:
				glUniform4fv( udv->sProgVar->getLoc(), 1, &udv->value.vec4[0] );
				break;
		}
	}
}



