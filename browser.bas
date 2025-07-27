Const sizes = 4096
Dim As String buffer, tagcheck
Dim As Integer inBody = 0, inScript = 0
Dim As Integer i, j, ch, insideTag = 0
Dim As String hrefText
Dim As String filename

' Perguntar o nome do ficheiro
Input "Nome do ficheiro HTML a ler: ", filename
Dim As Integer f = FreeFile
If Open(filename For Input As #f) <> 0 Then
    Print "Erro ao abrir o ficheiro!"
    End
End If

Color 0, 14
Cls

Do Until Eof(f)
    Line Input #f, buffer
    buffer = Lcase(buffer)
    i = 1
    Do While i <= Len(buffer)
        ch = Asc(Mid(buffer, i, 1))

        ' Verifica entrada em <body>
        If inBody = 0 Then
            If Instr(buffer, "<body") Then
                inBody = 1
                i = Instr(buffer, "<body") + 5
                Continue Do
            End If
        Else
            ' Dentro do body
            If Instr(Mid(buffer, i), "<script") Then
                inScript = 1
            ElseIf Instr(Mid(buffer, i), "</script") Then
                inScript = 0
            End If

            If inScript = 0 Then
                ' Ignorar tags
                If Mid(buffer, i, 1) = "<" Then
                    insideTag = 1

                    ' Detetar <br> ou <p>
                    If Instr(Mid(buffer, i), "<br") = 1 Or Instr(Mid(buffer, i), "<p") = 1 Or Instr(Mid(buffer, i), "</p") = 1 Then
                        Print
                    End If

                    ' Detetar href
                    If Instr(Mid(buffer, i), "href=") Then
                        j = Instr(Mid(buffer, i), "href=") + i - 1 + 5
                        If Mid(buffer, j, 1) = Chr(34) Then j += 1
                        hrefText = ""
                        Do While j <= Len(buffer) AndAlso Mid(buffer, j, 1) <> Chr(34)
                            hrefText += Mid(buffer, j, 1)
                            j += 1
                        Loop
                        Print "[LINK] "; hrefText
                    End If
                ElseIf Mid(buffer, i, 1) = ">" Then
                    insideTag = 0
                ElseIf insideTag = 0 Then
                    ' Imprimir texto normal fora das tags
                    Print Mid(buffer, i, 1);
                End If
            End If
        End If
        i += 1
    Loop
Loop

Close #f
