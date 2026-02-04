#pragma once

//Copyright (c) 2026 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN.  

#include <memory>

/// <summary>
/// Helper um statt einfachen pointer auf arrays mit pointer auf array mit größenangabe im datentyp arbeiten zu können
/// damit gehen dann auch range-based-for und bound check(zumindest ginge es, wenn es der compiler machen würde)
/// </summary>

namespace WS
{
	/// <summary>
	/// liefert die anzahl der elemente eines arrays
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="size_v"></typeparam>
	/// <param name="">ref auf array um das es geht</param>
	/// <returns>anzahl der elemente im array</returns>
	template<typename T,size_t size_v> constexpr size_t size( T (&)[size_v] ){ return size_v;}
	/// <summary>
	/// liefert die anzahl der elemente eines arrays
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="size_v"></typeparam>
	/// <param name="">pointer auf array um das es geht</param>
	/// <returns>anzahl der elemente im array</returns>
	template<typename T,size_t size_v> constexpr size_t size( T (*)[size_v] ){ return size_v;}
	//template<typename T,size_t size_v> consteval size_t size( T (&)[size_v] ){ return size_v;}
	//template<typename T,size_t size_v> consteval size_t size( T (*)[size_v] ){ return size_v;}

	/// <summary>
	/// liefert einen pointer auf T[size], dieser muss mit delete [] zerstört werden
	/// </summary>
	/// <typeparam name="T">typ der arrayelemente</typeparam>
	/// <typeparam name="...Args">typen der parameter für den konstruktor</typeparam>
	/// <typeparam name="size">die elementanzahl des array</typeparam>
	/// <param name="...args">parameter für den konstruktor</param>
	/// <returns></returns>
	template<typename T,size_t size,typename...Args> 
	auto MakeArray(Args...args){ return (T(*)[size]) new T[size]{std::forward<Args>(args)...}; }
	/// <summary>
	/// liefert einen std::unique_ptr auf T[size]
	/// allerdings kommt die falsche warning C4156 aus dem unique_ptr, dass delete statt delete[] aufgerufen wird, die Dtors werden trotzdem korrekt durchlaufen
	/// #pragma warning(suppress:4156) vor aufruf von MakeArrayPtr sollte das problem beheben
	/// </summary>
	/// <typeparam name="T">typ der arrayelemente</typeparam>
	/// <typeparam name="...Args">typen der parameter für den konstruktor</typeparam>
	/// <typeparam name="size">die elementanzahl des array</typeparam>
	/// <param name="...args">parameter für den konstruktor</param>
	/// <returns></returns>
	template<typename T,size_t size,typename...Args> 
	auto MakeArrayPtr(Args...args){ return std::unique_ptr<std::remove_pointer_t<T(*)[size]>>{ (T(*)[size]) new T[size]{std::forward<Args>(args)...} }; }
	

	/// <summary>
	/// legt eine array des types des ersten parameter an. dieser Parameter wird aber nicht verändert
	/// </summary>
	/// <typeparam name="T">typ der arrayelemente</typeparam>
	/// <typeparam name="...Args">typen der parameter für den T-konstruktor</typeparam>
	/// <typeparam name="size">elementanzahl des arrays</typeparam>
	/// <param name="ref">ein array das nur als typvorlage verwendet wird</param>
	/// <param name="parameter für den konstruktor...args"></param>
	/// <returns></returns>
	template<typename T,size_t size,typename...Args> 
	[[deprecated("deprecated: prefer MakeArrayPtr")]]
	auto MakeArray([[maybe_unused]]T(*ref)[size], Args...args){ return (T(*)[size]) new T[size]{std::forward<Args>(args)...}; }
	template<typename T,size_t size,typename...Args> 
	[[deprecated("deprecated: prefer MakeArrayPtr")]]
	auto MakeArray([[maybe_unused]]T(&ref)[size], Args...args){ return (std::remove_cv_t<T>(*)[size]) new std::remove_cv_t<T>[size]{std::forward<Args>(args)...}; }
	/// <summary>
	/// liefert einen std::unique_ptr auf T[size]
	/// allerdings kommt die falsche warning C4156 aus dem unique_ptr, dass delete statt delete[] aufgerufen wird, die Dtors werden trotzdem korrekt durchlaufen
	/// #pragma warning(suppress:4156) vor aufruf von MakeArrayPtr sollte das problem beheben
	/// </summary>
	/// <typeparam name="T">typ der arrayelemente</typeparam>
	/// <typeparam name="...Args">typen der parameter für den konstruktor</typeparam>
	/// <typeparam name="size">elementanzahl des arrays</typeparam>
	/// <param name="ref">ein array das nur als typvorlage verwendet wird</param>
	/// <param name="parameter für den konstruktor...args"></param>
	/// <returns></returns>
	template<typename T,size_t size,typename...Args> 
	auto MakeArrayPtr([[maybe_unused]]T(*ref)[size], Args...args){ return std::unique_ptr<std::remove_pointer_t<T(*)[size]>>{ (T(*)[size]) new T[size]{std::forward<Args>(args)...} }; }
	template<typename T,size_t size,typename...Args> 
	auto MakeArrayPtr([[maybe_unused]]T(&ref)[size], Args...args){ return std::unique_ptr<std::remove_pointer_t<std::remove_cv_t<T>(*)[size]>>{ (std::remove_cv_t<T>(*)[size]) new std::remove_cv_t<T>[size]{std::forward<Args>(args)...} }; }
}